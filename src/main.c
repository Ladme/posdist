// Released under MIT License.
// Copyright (c) 2022 Ladislav Bartos

#include <unistd.h>
#include <groan.h>

// frequency of printing during the calculation
static const int PROGRESS_FREQ = 10000;

/*
 * Parses command line arguments.
 * Returns zero, if parsing has been successful. Else returns non-zero.
 */
int get_arguments(
        int argc, 
        char **argv,
        char **gro_file,
        char **xtc_file,
        char **ndx_file,
        char **output_file,
        dimensionality_t *dim,
        char **selection1,
        char **selection2,
        int *timewise,
        int *whole,
        int *reference)
{
    int gro_specified = 0, selection1_specified = 0;

    int x = 0, y = 0, z = 0;

    int opt = 0;
    while((opt = getopt(argc, argv, "c:f:n:o:s:a:b:xyzhtwr")) != -1) {
        switch (opt) {
        // help
        case 'h':
            return 1;
        // gro file to read
        case 'c':
            *gro_file = optarg;
            gro_specified = 1;
            break;
        // xtc file to read
        case 'f':
            *xtc_file = optarg;
            break;
        // ndx file to read
        case 'n':
            *ndx_file = optarg;
            break;
        // output file
        case 'o':
            *output_file = optarg;
            break;
        // selection 1
        case 's':
        case 'a':
            *selection1 = optarg;
            selection1_specified = 1;
            break;
        // selection 2 (optional)
        case 'b':
            *selection2 = optarg;
            break;
        case 'x':
            x = 1;
            break;
        case 'y':
            y = 1;
            break;
        case 'z':
            z = 1;
            break;
        case 't':
            *timewise = 1;
            break;
        case 'w':
            *whole = 1;
            break;
        case 'r':
            *reference = 1;
            break;
        default:
            //fprintf(stderr, "Unknown command line option: %c.\n", opt);
            return 1;
        }
    }

    // set dimensions to calculate
    if (x + y + z == 0 || x + y + z == 3) *dim = dimensionality_xyz;
    else if (x + y == 2) *dim = dimensionality_xy;
    else if (x + z == 2) *dim = dimensionality_xz;
    else if (y + z == 2) *dim = dimensionality_yz;
    else if (x == 1)     *dim = dimensionality_x;
    else if (y == 1)     *dim = dimensionality_y;
    else                 *dim = dimensionality_z;

    if (*selection2 != NULL && *selection1 == NULL) {
        fprintf(stderr, "Selection 2 (-b) cannot be specified, unless selection 1 (-a / -s) is also specified.\n");
        return 1;
    }

    if (*reference && *selection2 == NULL) {
        fprintf(stderr, "Reference option (-r) was selected but there is no selection 2 to use as a reference.\n");
        return 1;
    }

    if (*timewise == 1 && *xtc_file == NULL) {
        fprintf(stderr, "Timewise option (-w) was selected but no xtc file was supplied.\n");
        return 1;
    }

    if (!gro_specified || !selection1_specified) {
        fprintf(stderr, "Gro file and at least one selection must always be supplied.\n");
        return 1;
    }

    if (*reference && *whole) {
        fprintf(stderr, "Reference option (-r) as well as whole option (-w) were selected. The reference option will be ignored.\n");
    }

    return 0;
}

void print_usage(const char *program_name)
{
    printf("Usage: %s -c GRO_FILE -s SELECTION1 [OPTION]...\n", program_name);
    printf("\nOPTIONS\n");
    printf("-h               print this message and exit\n");
    printf("-c STRING        gro file to read\n");
    printf("-f STRING        xtc file to read\n");
    printf("-n STRING        ndx file to read (optional, default: index.ndx)\n");
    printf("-s / -a STRING   selection of atoms\n");
    printf("-b STRING        another selection of atoms (optional)\n");
    printf("-o STRING        output file (default: posdist.dat)\n");
    printf("-x/-y/-z         dimension(s) that shall be treated (default: xyz)\n");
    printf("-t               calculate properties in time (optional)\n");
    printf("-w               calculate center of selection(s)\n");
    printf("-r               treat the selection -b as a reference point\n");
    printf("\n");
}

/*
 * Prints parameters that the program will use.
 */
void print_arguments(
        const char *gro_file,
        const char *xtc_file,
        const char *ndx_file,
        const char *output_file,
        const char *selection1,
        const char *selection2,
        const int timewise,
        const int whole,
        const int reference,
        const dimensionality_t dim)
{
    printf("\nParameters for PosDist calculation:\n");
    printf(">>> gro file:        %s\n", gro_file);
    if (xtc_file != NULL) printf(">>> xtc file:        %s\n", xtc_file);
    printf(">>> ndx file:        %s\n", ndx_file);
    if (!whole || timewise) printf(">>> output file:     %s\n", output_file);
    printf(">>> selection 1:     %s\n", selection1);
    if (selection2 != NULL) printf(">>> selection 2:     %s\n", selection2);
    
    switch (dim) {
    case dimensionality_xyz:
        printf(">>> dimensions:      xyz\n");
        break;
    case dimensionality_xy:
        printf(">>> dimensions:      xy\n");
        break;
    case dimensionality_xz:
        printf(">>> dimensions:      xz\n");
        break;
    case dimensionality_yz:
        printf(">>> dimensions:      yz\n");
        break;
    case dimensionality_x:
        printf(">>> dimensions:      x\n");
        break;
    case dimensionality_y:
        printf(">>> dimensions:      y\n");
        break;
    case dimensionality_z:
        printf(">>> dimensions:      z\n");
        break;
    default:
        printf(">>> dimensions:      UNKNOWN [this should never happen]\n");
        break;
    }

    if (timewise) printf(">>> timewise:        yes\n");
    else printf(">>> timewise:        no\n");
    if (whole) printf(">>> whole:           yes\n");
    else printf(">>> whole:           no\n");
    
    if (selection2 != NULL) {
        if (reference) printf(">>> reference:       yes\n");
        else printf(">>> reference:       no\n");
    }
    

    printf("\n");
}

/*! @brief Unpacks dimensionality. Returns 0, if successful, else returns 1. */
static int unpack_dimensionality(const dimensionality_t dim, int *x, int *y, int *z)
{
    switch (dim) {
    case dimensionality_xyz:
        *x = 1; *y = 1; *z = 1;
        break;
    case dimensionality_xy:
        *x = 1; *y = 1;
        break;
    case dimensionality_xz:
        *x = 1; *z = 1;
        break;
    case dimensionality_yz:
        *y = 1; *z = 1;
        break;
    case dimensionality_x:
        *x = 1;
        break;
    case dimensionality_y:
        *y = 1;
        break;
    case dimensionality_z:
        *z = 1;
        break;
    default:
        fprintf(stderr, "Internal unpack_dimensionality() error. Unknown dimensionality. This should never happen.\n");
        return 1;
    }

    return 0;
}

int calc_position(
        system_t *system, 
        const char *xtc_file, 
        const atom_selection_t *selection, 
        const char *selection_query, 
        const int timewise, 
        const int whole, 
        const dimensionality_t dim,
        const char *output_file)
{
    int x = 0, y = 0, z = 0;
    if (unpack_dimensionality(dim, &x, &y, &z) != 0) return 1;

    // if no xtc file is supplied, analyze the current state of the system
    if (xtc_file == NULL) {
        if (whole) {
            vec_t center = {0.0};
            center_of_geometry(selection, center, system->box);

            printf("Center of geometry of selection '%s': ", selection_query);
            if (x) printf("x = %.3f    ", center[0]);
            if (y) printf("y = %.3f    ", center[1]);
            if (z) printf("z = %.3f    ", center[2]);
            printf("\n");
        }

        else {
            // open output file
            FILE *output = fopen(output_file, "w");
            if (output == NULL) {
                fprintf(stderr, "Could not open output file '%s'\n", output_file);
                return 1;
            }

            fprintf(output, "Positions of atoms of selection '%s'.\n", selection_query);
            for (size_t i = 0; i < selection->n_atoms; ++i) {
                atom_t *atom = selection->atoms[i];
                fprintf(output, "Atom %s (id: %d) of residue %s (resid: %d):    ", atom->atom_name, atom->atom_number, atom->residue_name, atom->residue_number);
                if (x) fprintf(output, "x = %.3f    ", atom->position[0]);
                if (y) fprintf(output, "y = %.3f    ", atom->position[1]);
                if (z) fprintf(output, "z = %.3f    ", atom->position[2]);
                fprintf(output, "\n");
            }

            printf("Output file '%s' has been written.\n", output_file);
            fclose(output);
        }
    // if an xtc file is provided
    } else {
        // open and validate an xtc file
        XDRFILE *xtc = xdrfile_open(xtc_file, "r");
        if (xtc == NULL) {
            fprintf(stderr, "File %s could not be read as an xtc file.\n", xtc_file);
            return 1;
        }

        if (!validate_xtc(xtc_file, (int) system->n_atoms)) {
            fprintf(stderr, "Number of atoms in %s does not match the gro file.\n", xtc_file);
            return 1;
        }

        // calculate the center of geometry
        if (whole) {

            FILE *output = NULL;
            if (timewise) {
                output = fopen(output_file, "w");
                if (output == NULL) {
                    fprintf(stderr, "Could not open output file '%s'\n", output_file);
                    return 1;
                }

                fprintf(output, "Center of geometry of selection '%s' in time. \n", selection_query);
            }

            vec_t center = {0.0};
            vec_t av_center = {0.0};
            size_t n_steps = 0;

            while (read_xtc_step(xtc, system) == 0) {
                // print info about the progress of reading and writing
                if ((int) system->time % PROGRESS_FREQ == 0) {
                    printf("Step: %d. Time: %.0f ps\r", system->step, system->time);
                    fflush(stdout);
                }

                center_of_geometry(selection, center, system->box);

                if (timewise) {
                    fprintf(output, "t = %f    ", system->time);
                    if (x) fprintf(output, "x = %.3f    ", center[0]);
                    if (y) fprintf(output, "y = %.3f    ", center[1]);
                    if (z) fprintf(output, "z = %.3f    ", center[2]);
                    fprintf(output, "\n");
                } else {
                    av_center[0] += center[0];
                    av_center[1] += center[1];
                    av_center[2] += center[2];
                }

                ++n_steps;
            }

            if (timewise) {
                fclose(output);
            } else {
                printf("\nAverage center of geometry of selection '%s': ", selection_query);
                if (x) printf("x = %.3f    ", av_center[0] / n_steps);
                if (y) printf("y = %.3f    ", av_center[1] / n_steps);
                if (z) printf("z = %.3f    ", av_center[2] / n_steps);
                printf("\n");
            }
        
        // calculate position of each atom
        } else {
            FILE *output = fopen(output_file, "w");
            if (output == NULL) {
                fprintf(stderr, "Could not open output file '%s'\n", output_file);
                return 1;
            }

            if (timewise) fprintf(output, "Positions of atoms of selection '%s' in time. \n", selection_query);
            else fprintf(output, "Average positions of atoms of selection '%s'. \n", selection_query);

            vec_t *av_pos = calloc(selection->n_atoms, 3 * sizeof(float));
            size_t n_steps = 0;

            while (read_xtc_step(xtc, system) == 0) {
                // print info about the progress of reading and writing
                if ((int) system->time % PROGRESS_FREQ == 0) {
                    printf("Step: %d. Time: %.0f ps\r", system->step, system->time);
                    fflush(stdout);
                }

                if (timewise) fprintf(output, "t = %f\n", system->time);

                for (size_t i = 0; i < selection->n_atoms; ++i) {
                    atom_t *atom = selection->atoms[i];
                    if (timewise) {
                        fprintf(output, "Atom %s (id: %d) of residue %s (resid: %d):    ", atom->atom_name, atom->atom_number, atom->residue_name, atom->residue_number);
                        if (x) fprintf(output, "x = %.3f    ", atom->position[0]);
                        if (y) fprintf(output, "y = %.3f    ", atom->position[1]);
                        if (z) fprintf(output, "z = %.3f    ", atom->position[2]);
                        fprintf(output, "\n");
                    } else {
                        av_pos[i][0] += atom->position[0];
                        av_pos[i][1] += atom->position[1];
                        av_pos[i][2] += atom->position[2];
                    }
                }

                ++n_steps;
            } 

            if (!timewise) {
                for (size_t i = 0; i < selection->n_atoms; ++i) {
                    atom_t *atom = selection->atoms[i];
                    fprintf(output, "Atom %s (id: %d) of residue %s (resid: %d):    ", atom->atom_name, atom->atom_number, atom->residue_name, atom->residue_number);
                    if (x) fprintf(output, "x = %.3f    ", av_pos[i][0] / n_steps);
                    if (y) fprintf(output, "y = %.3f    ", av_pos[i][1] / n_steps);
                    if (z) fprintf(output, "z = %.3f    ", av_pos[i][2] / n_steps);
                    fprintf(output, "\n");
                }
            }

            free(av_pos);
            fclose(output);
        }

        xdrfile_close(xtc);

    }
    
    return 0;
}

/*! @brief Unpacks dimensionality. Returns 0, if successful, else returns 1. */
static int unpack_dimensionality_string(const dimensionality_t dim, char *string)
{
    switch (dim) {
    case dimensionality_xyz:
        strncpy(string, "xyz", 4);
        break;
    case dimensionality_xy:
        strncpy(string, "xy", 3);
        break;
    case dimensionality_xz:
        strncpy(string, "xz", 3);
        break;
    case dimensionality_yz: 
        strncpy(string, "yz", 3);
        break;
    case dimensionality_x:
        strncpy(string, "x", 2);
        break;
    case dimensionality_y:
        strncpy(string, "y", 2);
        break;
    case dimensionality_z:
        strncpy(string, "z", 2);
        break;
    default:
        fprintf(stderr, "Internal unpack_dimensionality_string() error. Unknown dimensionality. This should never happen.\n");
        return 1;
    }

    return 0;
}

int calc_distance(
        system_t *system,
        const char *xtc_file,
        const atom_selection_t *selection1,
        const atom_selection_t *selection2,
        const char *selection1_query,
        const char *selection2_query,
        const int timewise,
        const int whole,
        const int reference,
        const dimensionality_t dim,
        const char *output_file)
{
    char dimensions[4] = "";
    if (unpack_dimensionality_string(dim, dimensions) != 0) return 1;

    // if no xtc file is supplied, analyze the current state of the system
    if (xtc_file == NULL) {
        if (whole) {
            vec_t center1 = {0.0};
            vec_t center2 = {0.0};
            center_of_geometry(selection1, center1, system->box);
            center_of_geometry(selection2, center2, system->box);
            
            printf("%s-distance between the centers of selection '%s' and '%s': %.3f\n", dimensions, selection1_query, selection2_query, 
                    calc_distance_dim(center1, center2, dim, system->box, 1));
        } else {
            // open output file
            FILE *output = fopen(output_file, "w");
            if (output == NULL) {
                fprintf(stderr, "Could not open output file '%s'\n", output_file);
                return 1;
            }

            if (reference) {
                vec_t center2 = {0.0};
                center_of_geometry(selection2, center2, system->box);

                fprintf(output, "%s-distances between the atoms of selection '%s' and center of selection '%s'.\n", dimensions, selection1_query, selection2_query);
                for (size_t i = 0; i < selection1->n_atoms; ++i) {
                    atom_t *atom = selection1->atoms[i];
                    fprintf(output, "Atom %s (id: %d) of residue %s (resid: %d):    %.3f\n", atom->atom_name, atom->atom_number, atom->residue_name, atom->residue_number,
                            calc_distance_dim(atom->position, center2, dim, system->box, 1));
                }

            } else {

                fprintf(output, "%s-distances between the atoms of selections '%s' and '%s'.\n", dimensions, selection1_query, selection2_query);
                for (size_t i = 0; i < selection1->n_atoms; ++i) {
                    atom_t *atom1 = selection1->atoms[i];
                    fprintf(output, "Atom %s (id: %d) of residue %s (resid: %d):\n", atom1->atom_name, atom1->atom_number, atom1->residue_name, atom1->residue_number);
                    for (size_t j = 0; j < selection2->n_atoms; ++j) {
                        atom_t *atom2 = selection2->atoms[j];
                        fprintf(output, ">>> Atom %s (id: %d) of residue %s (resid: %d):   %.3f\n", atom2->atom_name, atom2->atom_number, atom2->residue_name, atom2->residue_number,
                                calc_distance_dim(atom1->position, atom2->position, dim, system->box, 1));
                    }
                }

            }
        }
    }

    return 0;

}

int main(int argc, char **argv)
{
    // get arguments
    char *gro_file = NULL;
    char *xtc_file = NULL;
    char *ndx_file = "index.ndx";
    char *output_file = "posdist.dat";
    char *selection1_query = NULL;
    char *selection2_query = NULL;
    int timewise = 0;
    int whole = 0;
    int reference = 0;
    dimensionality_t dim = dimensionality_xyz;

    if (get_arguments(argc, argv, &gro_file, &xtc_file, &ndx_file, &output_file, &dim, &selection1_query, &selection2_query, &timewise, &whole, &reference) != 0) {
        print_usage(argv[0]);
        return 1;
    }

    print_arguments(gro_file, xtc_file, ndx_file, output_file, selection1_query, selection2_query, timewise, whole, reference, dim);

    // read gro file
    system_t *system = load_gro(gro_file);
    if (system == NULL) return 1;

    // try reading ndx file (ignore if this fails)
    dict_t *ndx_groups = read_ndx(ndx_file, system);

    // select all atoms
    atom_selection_t *all = select_system(system);

    // select selection1
    atom_selection_t *selection1 = smart_select(all, selection1_query, ndx_groups);
    if (selection1 == NULL || selection1->n_atoms == 0) {
        fprintf(stderr, "No atoms ('%s') found.\n", selection1_query);

        dict_destroy(ndx_groups);
        free(all);
        free(system);
        free(selection1);
        return 1;
    }

    // if query for selection2 is supplied, select selection2 and calculate distances
    if (selection2_query != NULL) {
        atom_selection_t *selection2 = smart_select(all, selection2_query, ndx_groups);
        if (selection2 == NULL || selection2->n_atoms == 0) {
            fprintf(stderr, "No atoms ('%s') found.\n", selection2_query);

            dict_destroy(ndx_groups);
            free(all);
            free(system);
            free(selection1);
            free(selection2);
            return 1;
        }

        if (calc_distance(system, xtc_file, selection1, selection2, selection1_query, selection2_query, timewise, whole, reference, dim, output_file) != 0) {
            dict_destroy(ndx_groups);
            free(all);
            free(system);
            free(selection1);
            free(selection2);
            return 1;
        }

        free(selection2);
    // calculate position of selection1
    } else {
        if (calc_position(system, xtc_file, selection1, selection1_query, timewise, whole, dim, output_file) != 0) {
            dict_destroy(ndx_groups);
            free(all);
            free(system);
            free(selection1);
            return 1;
        }
    }

    printf("\n");

    dict_destroy(ndx_groups);
    free(all);
    free(system);
    free(selection1);
    return 0;
}