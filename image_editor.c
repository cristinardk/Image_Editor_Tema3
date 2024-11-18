//# Copyright Cristina Iordache 314CAa 2023-2024
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct {
    unsigned char pixel_val;
} Pixel_grayscale;

typedef struct {
    Pixel_grayscale** img_matrix;
}Gray_Image;

typedef struct {
    unsigned char red_val;
    unsigned char green_val;
    unsigned char blue_val;
} Pixel_color;

typedef struct {
    Pixel_color** img_matrix;
} Color_Image;

typedef struct {
    char* filename;
    int dim;
    int lines;
    int cols;
    Gray_Image gray_img;
    Color_Image color_img;
    int x1, y1, x2, y2;
} Image;

double clamp(double value) {
    return fmax(0.0, fmin(255.0, value));
}

void free_image(Image* img) {
    if (img->dim == 1) {
        for (int i = 0; i < img->lines; i++)
            free(img->gray_img.img_matrix[i]);
        free(img->gray_img.img_matrix);
    }
    else if (img->dim == 3) {
        for (int i = 0; i < img->lines; i++)
            free(img->color_img.img_matrix[i]);
        free(img->color_img.img_matrix);
    }
    img->color_img.img_matrix = NULL;
    img->gray_img.img_matrix = NULL;
    img->dim = 0;
    img->lines = 0;
    img->cols = 0;
    img->x1 = 0;
    img->y1 = 0;
    img->x2 = 0;
    img->y2 = 0;
}
void exit_im(Image* img) {
    if (!(img->gray_img.img_matrix) && !(img->color_img.img_matrix)) {
        printf("No image loaded\n");
    }
        if (img->dim == 1) {
            for (int i = 0; i < img->lines; i++)
                free(img->gray_img.img_matrix[i]);
            free(img->gray_img.img_matrix);
        }
        else if (img->dim == 3) {
            for (int i = 0; i < img->lines; i++)
                free(img->color_img.img_matrix[i]);
            free(img->color_img.img_matrix);
        }
        free(img); 
}

void skip_comm(FILE* file) {
    char c;
    c = fgetc(file);
    while (c != 26 && c == '#') {
        do {
            c = fgetc(file);
        } while (c != '\n');
        c = fgetc(file);
    }
    fseek(file, -1, SEEK_CUR);
}

void load(Image* img) {
    if (img != NULL) {
        free_image(img);
    }
    FILE* fin = fopen(img->filename, "rb");
    if (!fin) {
            printf("Failed to load %s\n", img->filename);
            return;
    }
    int isBinary;
    int dim;
    char* line = NULL;
    size_t* line_size = malloc(sizeof(size_t));
    skip_comm(fin);
    getline(&line, line_size, fin);
    if (line[0] != 'P') {
        fprintf(stderr, "Invalid magic number in the file\n");
        return;
    }

    if (line[1] == '2') {
        dim = 1;
        isBinary = 0;
    }
    else if (line[1] == '3') {
        dim = 3;
        isBinary = 0;
    }
    else if (line[1] == '5') {
        dim = 1;
        isBinary = 1;
    }
    else if (line[1] == '6') {
        dim = 3;
        isBinary = 1;
    }
    else {
        fprintf(stderr, "Invalid magic number in the file\n");
        return;
    }

    int lines, cols, color_num;
    skip_comm(fin);
    fscanf(fin, "%d %d ", &cols, &lines);

    skip_comm(fin);
    fscanf(fin, "%d ", &color_num);

    if (color_num != 255) {
        fprintf(stderr, "Failed to load image\n");
        return;
    }

    skip_comm(fin);

    if (dim == 1) {
        // Inițializează o imagine în tonuri de gri
        img->color_img.img_matrix = NULL;
        img->dim = 1;
        img->lines = lines;
        img->cols = cols;
        img->gray_img.img_matrix = (Pixel_grayscale**)malloc(lines * sizeof(Pixel_grayscale*));
        for (int i = 0; i < lines; i++) {
            img->gray_img.img_matrix[i] = (Pixel_grayscale*)malloc(cols * sizeof(Pixel_grayscale));
        }
    }
    else if (dim == 3) {
        // Inițializează o imagine color
        img->gray_img.img_matrix = NULL;
        img->dim = 3;
        img->lines = lines;
        img->cols = cols;
        img->color_img.img_matrix = (Pixel_color**)malloc(lines * sizeof(Pixel_color*));
        for (int i = 0; i < lines; i++) {
            img->color_img.img_matrix[i] = (Pixel_color*)malloc(cols * sizeof(Pixel_color));
        }
    }

    unsigned char val;
    for (int i = 0; i < lines; i++) {
        for (int j = 0; j < cols; j++) {
            if (dim == 1) {
                if (isBinary)
                    fread(&val, sizeof(char), 1, fin);
                else fscanf(fin, "%hhu", &val);
                img->gray_img.img_matrix[i][j].pixel_val = val;
            }
            else if (dim == 3) {
                if (isBinary)
                    fread(&val, sizeof(char), 1, fin);
                else fscanf(fin, "%hhu", &val);
                img->color_img.img_matrix[i][j].red_val = val;

                if (isBinary)
                    fread(&val, sizeof(char), 1, fin);
                else fscanf(fin, "%hhu", &val);
                img->color_img.img_matrix[i][j].green_val = val;

                if (isBinary)
                    fread(&val, sizeof(char), 1, fin);
                else fscanf(fin, "%hhu", &val);
                img->color_img.img_matrix[i][j].blue_val = val;
            }
        }
    }

    img->x1 = 0;
    img->y1 = 0;
    img->x2 = cols;
    img->y2 = lines;

    free(line);
    free(line_size);
    printf("Loaded %s\n", img->filename);

    fclose(fin);
}


void swap(int* a, int* b) {
    int aux;
    aux = *a;
    *a = *b;
    *b = aux;
}

void select_im(Image* img, int x1, int y1, int x2, int y2) {
    if (!(img->gray_img.img_matrix) && !(img->color_img.img_matrix)) {
        printf("No image loaded\n");
        return;
    }

    img->x1 = x1;
    img->x2 = x2;
    img->y1 = y1;
    img->y2 = y2;
    if (img->x1 > img->x2)
        swap(&(img->x1), &(img->x2));
    if (img->y1 > img->y2)
        swap(&(img->y1), &(img->y2));

    printf("Selected %d %d %d %d\n", img->x1, img->y1, img->x2, img->y2);
}

void select_all_im(Image* img) {
    if (!(img->gray_img.img_matrix) && !(img->color_img.img_matrix)) {
        printf("No image loaded\n");
        return;
    }
    img->x1 = 0;
    img->x2 = img->cols;
    img->y1 = 0;
    img->y2 = img->lines;
    printf("Selected ALL\n");
}

Image* crop(Image* img) {
    if (!(img->gray_img.img_matrix) && !(img->color_img.img_matrix)) {
        printf("No image loaded\n");
        return img;
    }
    // Alocă memorie pentru img_copy
    Image *img_copy = malloc(sizeof(Image));
    img_copy->dim = img->dim;
    img_copy->lines = img->y2 - img->y1;
    img_copy->cols = img->x2 - img->x1;
    img_copy->x1 = 0;
    img_copy->x2 = img_copy->cols;
    img_copy->y1 = 0;
    img_copy->y2 = img_copy->lines;

    if (img_copy->dim == 1) {
        // Inițializează o imagine în tonuri de gri
        img_copy->gray_img.img_matrix = (Pixel_grayscale**)malloc(img_copy->lines * sizeof(Pixel_grayscale*));
        for (int i = 0; i < img_copy->lines; i++) {
            img_copy->gray_img.img_matrix[i] = (Pixel_grayscale*)calloc(img_copy->cols, sizeof(Pixel_grayscale));
        }
    }
    else if (img_copy->dim == 3) {
        // Inițializează o imagine color
        img_copy->color_img.img_matrix = (Pixel_color**)malloc(img_copy->lines * sizeof(Pixel_color*));
        for (int i = 0; i < img_copy->lines; i++) {
            img_copy->color_img.img_matrix[i] = (Pixel_color*)calloc(img_copy->cols, sizeof(Pixel_color));
        }
    }

    // Copiază pixel cu pixel imaginea selectată în img_copy
    for (int i = 0; i < img_copy->lines; i++) {
        for (int j = 0; j < img_copy->cols; j++) {
            if (img_copy->dim == 1) {
                img_copy->gray_img.img_matrix[i][j].pixel_val = img->gray_img.img_matrix[img->y1 + i][img->x1 + j].pixel_val;
            }
            else if (img_copy->dim == 3) {
                img_copy->color_img.img_matrix[i][j].red_val = img->color_img.img_matrix[img->y1 + i][img->x1 + j].red_val;
                img_copy->color_img.img_matrix[i][j].green_val = img->color_img.img_matrix[img->y1 + i][img->x1 + j].green_val;
                img_copy->color_img.img_matrix[i][j].blue_val = img->color_img.img_matrix[img->y1 + i][img->x1 + j].blue_val;
            }
        }
    }

    free_image(img);
    free(img);

    return img_copy;
}

void generate_histogram(Image* img, int x, int y) {

    if (!(img->gray_img.img_matrix) && !(img->color_img.img_matrix)) {
        printf("No image loaded\n");
        return;
    }
    if (img->dim > 1) {
        printf("Black and white image needed\n");
        return;
    }

    if (y < 2 || y > 256 || (y & (y - 1)) != 0) {
        printf("Invalid number of bins. Must be a power of 2 in the range [2, 256]\n");
        return;
    }

    int *histogram;
    histogram = (int *)calloc(256, sizeof(int));
    int *vec_sum;
    vec_sum = (int *)calloc(y, sizeof(int));


    // Calcularea histograma
    for (int i = 0; i < img->lines; i++) {
        for (int j = 0; j < img->cols; j++) {
            histogram[img->gray_img.img_matrix[i][j].pixel_val]++;
        }
    }
        int k = 0;
        for(int i = 0; i < y; i++) {
            for(int j = 0; j < 256/y; j++) {
                vec_sum[i] += histogram[k++];
            }
        }

    int maxFrequency = 0;
    for (int i = 0; i < y; i++) {
        if (vec_sum[i] > maxFrequency) {
            maxFrequency = vec_sum[i];
        }
    }
    int stars = 0;
    // Afisarea histograma
    for (int i = 0; i < y; i++) {
        stars = (x * vec_sum[i]) / maxFrequency;
        printf("%d\t|\t", stars);
        for (int k = 0; k < stars; k++) {
            printf("*");
        }
        printf("\n");
    }
    free(histogram);
    free(vec_sum);
}

void equalize(Image* img) {
    if (!(img->gray_img.img_matrix) && !(img->color_img.img_matrix)) {
        printf("No image loaded\n");
        return;
    }
    if (img->dim > 1) {
        printf("Black and white image needed\n");
        return;
    }
    int *frc;
    frc = (int *)calloc(256, sizeof(int));
    for (int i = 0; i < img->lines; i++) {
        for (int j = 0; j < img->cols; j++) {
            frc[(int)img->gray_img.img_matrix[i][j].pixel_val]++;
        }
    }


    for (int i = 1; i < 256; i++)
        frc[i] += frc[i - 1];
    int area = img->lines * img->cols;

    for (int i = 0; i < img->lines; i++) {
        for (int j = 0; j < img->cols; j++) {
            img->gray_img.img_matrix[i][j].pixel_val = (unsigned char)(clamp((round)(frc[(int)(img->gray_img.img_matrix[i][j].pixel_val)] * 255.0 / area)));
        }
    }
    free(frc);
    printf("Equalize done\n");
}

void save_im(Image* img, char* p) {
    int is_ascii;
    FILE* fout;
    if (p != NULL)
        is_ascii = (strcmp(p, "ascii") == 0);
    else
        is_ascii = 0;
    
    if (is_ascii)
        fout = fopen(img->filename, "w");
    else {
		fout = fopen(img->filename, "wb");
	}


    if (!fout) {
        printf("Failed to save to %s\n", img->filename);
        return;
    }

    if (img->dim == 1) {
        if (is_ascii) {
            fprintf(fout, "P2\n");
            fprintf(fout, "%d %d\n", img->cols, img->lines);
            fprintf(fout, "255\n");

            for (int i = 0; i < img->lines; i++) {
                for (int j = 0; j < img->cols; j++) {
                    fprintf(fout, "%d ", img->gray_img.img_matrix[i][j].pixel_val);
                }
                fprintf(fout, "\n");
            }
        }
        else {
            fprintf(fout, "P5\n");
            fprintf(fout, "%d %d\n", img->cols, img->lines);
            fprintf(fout, "255\n");

            for (int i = 0; i < img->lines; i++) {
                for (int j = 0; j < img->cols; j++) {
                    fputc(img->gray_img.img_matrix[i][j].pixel_val, fout);
                }
            }
        }
    }
    else if (img->dim == 3) {
        if (is_ascii) {
            fprintf(fout, "P3\n");
            fprintf(fout, "%d %d\n", img->cols, img->lines);
            fprintf(fout, "255\n");

            for (int i = 0; i < img->lines; i++) {
                for (int j = 0; j < img->cols; j++) {
                    fprintf(fout, "%hhu %hhu %hhu ",
                        img->color_img.img_matrix[i][j].red_val,
                        img->color_img.img_matrix[i][j].green_val,
                        img->color_img.img_matrix[i][j].blue_val);
                }
                fprintf(fout, "\n");
            }
        }
        else {
            fprintf(fout, "P6\n");
            fprintf(fout, "%d %d\n", img->cols, img->lines);
            fprintf(fout, "255\n");

            for (int i = 0; i < img->lines; i++) {
                for (int j = 0; j < img->cols; j++) {
                    fputc(img->color_img.img_matrix[i][j].red_val, fout);
                    fputc(img->color_img.img_matrix[i][j].green_val, fout);
                    fputc(img->color_img.img_matrix[i][j].blue_val, fout);
                }
            }
        }
    }

    fclose(fout);
    printf("Saved %s\n", img->filename);
}

void apply_filter(Image *img, const double kernel_matrix[3][3]) {
    if (img->dim != 3) {
        printf("Easy, Charlie Chaplin\n");
        return;
    }
    Image *img_result = malloc(sizeof(Image));
    img_result->dim = img->dim;
    img_result->lines = img->lines;
    img_result->cols = img->cols;
    img_result->x1 = img->x1;
    img_result->x2 = img->x2;
    img_result->y1 = img->y1;
    img_result->y2 = img->y2;

    // Inițializează o imagine color
    img_result->color_img.img_matrix = (Pixel_color**)malloc(img->lines * sizeof(Pixel_color*));
    for (int i = 0; i < img->lines; i++) {
        img_result->color_img.img_matrix[i] = (Pixel_color*)malloc(img->cols * sizeof(Pixel_color));
    }
        for(int i = 0; i < img->lines; i++) {
            for(int j = 0; j < img->cols; j++) {
                img_result->color_img.img_matrix[i][j].red_val = img->color_img.img_matrix[i][j].red_val;
                img_result->color_img.img_matrix[i][j].green_val = img->color_img.img_matrix[i][j].green_val;
                img_result->color_img.img_matrix[i][j].blue_val = img->color_img.img_matrix[i][j].blue_val;
            }
        }
        for(int i = img->y1; i < img->y2; i++) {
            for(int j = img->x1; j < img->x2; j++) {
                if(i > 0 && j > 0 && i < img->lines - 1 && j < img->cols - 1) {
                    double pixel_value1 = 0, pixel_value2 = 0, pixel_value3 = 0;
                    for(int m = 0; m < 3; m++) {
                        for(int n = 0; n < 3; n++) {
                            pixel_value1 += (double)(img->color_img.img_matrix[i + m - 1][j + n - 1].red_val) * (double)kernel_matrix[m][n];
                            pixel_value2 += (double)(img->color_img.img_matrix[i + m - 1][j + n - 1].green_val) * (double)kernel_matrix[m][n];
                            pixel_value3 += (double)(img->color_img.img_matrix[i + m - 1][j + n - 1].blue_val) * (double)kernel_matrix[m][n];
                        }
                    }
                    pixel_value1 = clamp(pixel_value1);
                    pixel_value2 = clamp(pixel_value2);
                    pixel_value3 = clamp(pixel_value3); 

                    img_result->color_img.img_matrix[i][j].red_val = (unsigned char)((round)(pixel_value1));
                    img_result->color_img.img_matrix[i][j].green_val = (unsigned char)((round)(pixel_value2));
                    img_result->color_img.img_matrix[i][j].blue_val = (unsigned char)((round)(pixel_value3));
                }
            }
        }
        //Am copiat pixelii schimbati in img;
        for(int i = img->y1; i < img->y2; i++) {
            for(int j = img->x1; j < img->x2; j++) {
                img->color_img.img_matrix[i][j].red_val = (unsigned char)(img_result->color_img.img_matrix[i][j].red_val);
                img->color_img.img_matrix[i][j].green_val = (unsigned char)(img_result->color_img.img_matrix[i][j].green_val);
                img->color_img.img_matrix[i][j].blue_val = (unsigned char)(img_result->color_img.img_matrix[i][j].blue_val);
            }    
        }
        free_image(img_result);
        free(img_result);
}

/*Image* rotate_image(Image* img, int grade) {
    Image *img_copy = malloc(sizeof(Image));
    img_copy->dim = img->dim;
    int ok = 0;
    if(img->x1 == 0 && img->x2 == img->cols && img->y1 == 0 && img->y2 == img->lines)
        ok = 1;
    if(ok == 0) {
        img_copy->lines = img->lines;
        img_copy->cols = img->cols;
        img_copy->x1 = img->x1;
        img_copy->x2 = img->x2;
        img_copy->y1 = img->y1;
        img_copy->y2 = img->y2;
        if (img_copy->dim == 1) {
            img_copy->gray_img.img_matrix = (Pixel_grayscale**)malloc(img->lines * sizeof(Pixel_grayscale*));
            for (int i = 0; i < img->lines; i++) {
                img_copy->gray_img.img_matrix[i] = (Pixel_grayscale*)calloc(img->cols, sizeof(Pixel_grayscale));

            }
        }
        else if (img_copy->dim == 3) {
            img_copy->color_img.img_matrix = (Pixel_color**)malloc(img->lines * sizeof(Pixel_color*));
            for (int i = 0; i < img->lines; i++) {
                img_copy->color_img.img_matrix[i] = (Pixel_color*)calloc(img->cols, sizeof(Pixel_color));
            }
        }
        for(int i = 0; i < img_copy->lines; i++) {
            for(int j = 0; j < img_copy->cols; j++) {   
                if(img->dim == 1)
                    img_copy->gray_img.img_matrix[i][j].pixel_val = img->gray_img.img_matrix[i][j].pixel_val;
                else if (img->dim == 3) {
                    img_copy->color_img.img_matrix[i][j].red_val = img->color_img.img_matrix[i][j].red_val;
                    img_copy->color_img.img_matrix[i][j].green_val = img->color_img.img_matrix[i][j].green_val;
                    img_copy->color_img.img_matrix[i][j].blue_val = img->color_img.img_matrix[i][j].blue_val;
                }
            }
        }
            for (int i = img_copy->y1; i < img_copy->y2; i++) {
                for (int j = img_copy->x1; j < img_copy->x2; j++) {
                    if (img_copy->dim == 1) {
                        img_copy->gray_img.img_matrix[j][img->y2 - i - 1].pixel_val = img->gray_img.img_matrix[i][j].pixel_val;
                    }
                    else if (img_copy->dim == 3) {
                        img_copy->color_img.img_matrix[j][img->y2 - i - 1].red_val = img->color_img.img_matrix[i][j].red_val;
                        img_copy->color_img.img_matrix[j][img->y2 - i - 1].green_val = img->color_img.img_matrix[i][j].green_val;
                        img_copy->color_img.img_matrix[j][img->y2 - i - 1].blue_val = img->color_img.img_matrix[i][j].blue_val;
                    }
                }
            }
         
    } else {
        img_copy->dim = img->dim;
        img_copy->lines = img->cols;
        img_copy->cols = img->lines;
        img_copy->x1 = img->y1;
        img_copy->x2 = img->y2;
        img_copy->y1 = img->x1;
        img_copy->y2 = img->x2;

            if (img_copy->dim == 1) {
            img_copy->gray_img.img_matrix = (Pixel_grayscale**)malloc(img_copy->lines * sizeof(Pixel_grayscale*));
            for (int i = 0; i < img_copy->lines; i++) {
                img_copy->gray_img.img_matrix[i] = (Pixel_grayscale*)calloc(img_copy->cols, sizeof(Pixel_grayscale));
            }
            }
            else if (img_copy->dim == 3) {
                img_copy->color_img.img_matrix = (Pixel_color**)malloc(img_copy->lines * sizeof(Pixel_color*));
                for (int i = 0; i < img_copy->lines; i++) {
                    img_copy->color_img.img_matrix[i] = (Pixel_color*)calloc(img_copy->cols, sizeof(Pixel_color));
                }
            }
            for(int i = 0; i < img_copy->lines; i++) {
                for(int j = 0; j < img_copy->cols; j++) {   
                    if(img->dim == 1)
                        img_copy->gray_img.img_matrix[i][j].pixel_val = img->gray_img.img_matrix[j][i].pixel_val;
                    else if (img->dim == 3) {
                        img_copy->color_img.img_matrix[i][j].red_val = img->color_img.img_matrix[j][i].red_val;
                        img_copy->color_img.img_matrix[i][j].green_val = img->color_img.img_matrix[j][i].green_val;
                        img_copy->color_img.img_matrix[i][j].blue_val = img->color_img.img_matrix[j][i].blue_val;
                    }
                }
            }
            for (int i = img_copy->y1; i < img_copy->y2; i++) {
                for (int j = img_copy->x1; j < img_copy->x2; j++) {
                    if (img_copy->dim == 1) {
                        img_copy->gray_img.img_matrix[i][img_copy->x2 - j - 1].pixel_val = img->gray_img.img_matrix[j][i].pixel_val;
                    }
                    else if (img_copy->dim == 3) {
                        img_copy->color_img.img_matrix[i][img_copy->x2 - j - 1].red_val = img->color_img.img_matrix[j][i].red_val;
                        img_copy->color_img.img_matrix[i][img_copy->x2 - j - 1].green_val = img->color_img.img_matrix[j][i].green_val;
                        img_copy->color_img.img_matrix[i][img_copy->x2 - j - 1].blue_val = img->color_img.img_matrix[j][i].blue_val;
                    }
                }
            }
    }
        free_image(img); 
    free(img);
    return img_copy;
}
*/
Image* process_command(Image* img, char* command) {
    const double filters[4][3][3] = {
		{
			{-1, -1, -1},
			{-1, 8, -1},
			{-1, -1, -1}
		},
		{
			{0, -1, 0},
			{-1, 5, -1},
			{0, -1, 0}
		},
		{
			{1. / 9, 1. / 9, 1. / 9},
			{1. / 9, 1. / 9, 1. / 9},
			{1. / 9, 1. / 9, 1. / 9}
		},
		{
			{1. / 16, 2. / 16, 1. / 16},
			{2. / 16, 4. / 16, 2. / 16},
			{1. / 16, 2. / 16, 1. / 16}
		}
	};
    
    char* p = strtok(command, " \n");
    if (p == NULL) {
        return img;
        printf("Invalid command\n");
    }
    else if (strcmp(p, "LOAD") == 0) {
        p = strtok(NULL, " \n");
        if(p == NULL)
            printf("Invalid command\n");
        else{
            img->filename = p;
            load(img);
        }   
    }
    else if (strcmp(p, "SELECT") == 0) {
        int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        int vals[4] = {0};
        int cnt = 0;
        int is_number = 0;
        int ok = 1;
        p = strtok(NULL, " \n");
        if(strcmp(p, "ALL") == 0) {
            if(img->dim == 0)
                printf("No image loaded\n");
            else
                select_all_im(img);
        }
        else {
            while(p != NULL && ok) {
                if(p[0] == '-') {
                    for (int i = 1; p[i] != '\0'; i++) {
                        if (isdigit(p[i])) {
                            is_number++;
                        }
                    }
                    if (is_number == (int)(strlen(p) - 1)) {
                        vals[cnt++] = -atoi(p + 1);
                        p = strtok(NULL, " \n");
                        is_number = 0;
                    }
                    else
                        ok = 0;
                }
                else {
                    for (int i = 0; p[i] != '\0'; i++) {
                        if (isdigit(p[i])) {
                            is_number++;
                        }
                    }
                    if (is_number == (int)(strlen(p))) {
                            vals[cnt++] = atoi(p);
                            p = strtok(NULL, " \n");
                            is_number = 0;
                        }
                    else
                        ok = 0;
                }
            }
            if(ok && cnt == 4) {
                if(img->dim == 0)
                    printf("No image loaded\n");
                else {
                    x1 = vals[0];
                    y1 = vals[1];   
                    x2 = vals[2];
                    y2 = vals[3];
                    if (x1 < 0 || x1 >= img->cols || y1 < 0 || y1 >= img->lines || x2 < 0 || x2 > img->cols || y2 > img->lines || y2 < 0 || x1 == x2 || y1 == y2)                
                        printf("Invalid set of coordinates\n");
                    else
                        select_im(img, x1, y1, x2, y2);
                }
            }
            else 
                printf("Invalid command\n");
        }
    }
    else if (strcmp(p, "CROP") == 0) {
        if(img->dim == 0) {
            printf("No image loaded\n");
        }  
        else {
            img = crop(img);
            printf("Image cropped\n");
        }
    }
    else if (strcmp(p, "HISTOGRAM") == 0) {
        int is_number = 0;
        int ok = 1;
        int vals[3] = {0};
        int cnt = 0;
        p = strtok(NULL, " \n");

        if (p == NULL && img->dim == 0)
            printf("No image loaded\n");
        else if(p == NULL)
            printf("Invalid command\n");
        else {
            while(p != NULL && ok) {
                for (int i = 0; p[i] != '\0'; i++) {
                        if (isdigit(p[i])) {
                            is_number++;
                        }
                    }
                if(is_number == (int)(strlen(p))) {
                    vals[cnt++] = atoi(p);
                    p = strtok(NULL, " \n");
                    is_number = 0;
                }
                else
                    ok = 0;
            } if(ok && cnt == 2)
                generate_histogram(img, vals[0], vals[1]);
            else
                printf("Invalid command\n");
        }
    }
    else if (strcmp(p, "EQUALIZE") == 0) {
        p = strtok(NULL, " \n");
        if (p != NULL)
            printf("Invalid command\n");
        else if(img->dim == 0)
            printf("No image loaded\n");
        else 
            equalize(img);
    }
    else if (strcmp(p, "EXIT") == 0) {
        p = strtok(NULL, " \n");
        if(p != NULL)
            printf("Invalid command\n");
        else
            exit_im(img);
    }
    else if (strcmp(p, "SAVE") == 0) {
        p = strtok(NULL, " \n"); 
        if (p == NULL)
            printf("Invalid command\n");
        
        else if(img->dim == 0) {
            printf("No image loaded\n");
        } else {
            img->filename = p;
            p = strtok(NULL, " \n");
            save_im(img, p);
        } 

    } else if (strcmp(p, "APPLY") == 0) {
        if(img->dim == 0) {
            printf("No image loaded\n");
        } else {
            p = strtok(NULL, " \n");
            if (p == NULL)
                printf("Invalid command\n");
            else if(strcmp(p, "EDGE") == 0){
                    apply_filter(img, filters[0]);
                    if(img->color_img.img_matrix)
                        printf("APPLY EDGE done\n");
                }
            else if(strcmp(p, "SHARPEN") == 0){
                apply_filter(img, filters[1]);
                if(img->color_img.img_matrix)
                    printf("APPLY SHARPEN done\n");
            }
            else if(strcmp(p, "BLUR") == 0) {
                apply_filter(img, filters[2]);
                if(img->color_img.img_matrix)
                    printf("APPLY BLUR done\n");
            }
            else if(strcmp(p, "GAUSSIAN_BLUR") == 0) {
                apply_filter(img, filters[3]);
                if(img->color_img.img_matrix)
                    printf("APPLY GAUSSIAN_BLUR done\n");
            }
            else {
                printf("APPLY parameter invalid\n");
            }
        }
    } else if(strcmp(p, "ROTATE") == 0) {
        int is_number = 0, ok = 1;
        int grade;
        if(img->dim == 0)
                printf("No image loaded\n");
        else {
            p = strtok(NULL, " \n");
            while(p != NULL && ok) {
                if(p[0] == '-') {
                    for (int i = 1; p[i] != '\0'; i++) {
                        if (isdigit(p[i])) {
                            is_number++;
                        }
                    }
                    if (is_number == (int)(strlen(p) - 1)) {
                        grade = -atoi(p + 1);
                        p = strtok(NULL, " \n");
                        is_number = 0;
                    }
                    else
                        ok = 0;
                }
                else {
                    for (int i = 0; p[i] != '\0'; i++) {
                        if (isdigit(p[i])) {
                            is_number++;
                        }
                    }
                    if (is_number == (int)(strlen(p))) {
                            grade = atoi(p);
                            p = strtok(NULL, " \n");
                            is_number = 0;
                        }
                    else
                        ok = 0;
                }
            } if(ok) {
                if(grade % 90 || grade < -90 || grade > 360)
                    printf("Unsupported rotation angle\n");
                else if((img->x2 - img->x1 != img->y2 - img->y1) &&
                        (img->x1 != 0 && img->x2 != img->cols && img->y1 != 0 && img->y2 != img->lines))
                    printf("The selection must be square\n");
                
                /*else {
                    int grade_aux = grade;
                    if(grade < 0)
                        grade += 360;
                    while(grade) {
                        img = rotate_image(img, grade);
                        grade -= 90;
                    }
                    printf("Rotated %d\n", grade_aux);
                }*/
            } else
                printf("Invalid command\n");
        }
    } else
        printf("Invalid command\n");
    return img; 
}

int main() {
    char command[1000];
    char *cuv;
    Image* img = malloc(sizeof(Image));
    img->x1 = 0;
    img->x2 = 0;
    img->y1 = 0;
    img->y2 = 0;
    img->cols = 0;
    img->lines = 0;
    img->dim = 0;
    while (1) {
        fgets(command, sizeof(command), stdin);
        img = process_command(img, command);
        cuv = strtok(command, " \n");
        if(strcmp(cuv, "EXIT") == 0)
            break;
    }
}
