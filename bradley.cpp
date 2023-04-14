/*
 UWAGA - poni�szy kod traktujemmy tylko jako przyk�adowy,
specjalnie jest on nie do ko�ca kompletny i usuni�te sa chocia�by
fragmenty dotycz�ce zwalniania pami�cia, czy jej alokacji
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include "Ttiming.h"
#include "TPGM.h"

TTiming tt; // klasa do mierzenia czasu wykonywania si� poszczeg�lnych funkcji

bool replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void bradley_binarization(unsigned char **image, unsigned char **output, int width, int height, float threshold, int window_size)
{
    int x, y, i, j, p, sum, count, black, white;
    int half_window = window_size / 2;
    float mean, variance, std_dev, pixel_value, local_threshold;

    // Apply the thresholding
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            // Compute the mean of the surrounding window
            sum = 0;
            count = 0;
            for (j = -half_window; j <= half_window; j++)
            {
                for (i = -half_window; i <= half_window; i++)
                {
                    if (x + i >= 0 && x + i < width && y + j >= 0 && y + j < height)
                    {
                        sum += image[(y + j)][(x + i)];
                        count++;
                    }
                }
            }
            mean = (float)sum / count;

            // Compute the standard deviation of the surrounding window
            sum = 0;
            for (j = -half_window; j <= half_window; j++)
            {
                for (i = -half_window; i <= half_window; i++)
                {
                    if (x + i >= 0 && x + i < width && y + j >= 0 && y + j < height)
                    {
                        sum += pow(image[(y + j)][(x + i)] - mean, 2);
                    }
                }
            }
            variance = (float)sum / count;
            std_dev = sqrt(variance);

            // Binarize the pixel
            local_threshold = mean * (1 - threshold);
            output[y][x] = (image[y][x] >= local_threshold) ? 255 : 0;
        }
    }
}

int main(int argc, char **argv)
{
    int rows, cols;
    int max_color;
    int hpos, i, j;

    std::string infname = "img/maly.pgm";

    if ((hpos = readPGMB_header(infname.c_str(), &rows, &cols, &max_color)) <= 0)
        exit(1);

    unsigned char **a = new unsigned char *[rows];

    a[0] = new unsigned char[rows * cols];

    for (int i = 1; i < rows; i++)
    {
        a[i] = a[i - 1] + cols;
    }

    if (readPGMB_data(a[0], infname.c_str(), hpos, rows, cols, max_color) == 0)
        exit(1);

    // przygotowanie czarno-bialej tablicy wyjsciowej
    // analogicznie jak dla tablicy wejsciowej
    unsigned char **b = new unsigned char *[rows];
    b[0] = new unsigned char[rows * cols];

    for (int i = 1; i < rows; i++)
    {
        b[i] = b[i - 1] + cols;
    }

    tt.Begin(); // start to measure the time

    bradley_binarization(a, b, cols, rows, 0.1, 20);

    double elapsed = tt.End(); // stop and read elapsed time in ms (miliseconds)
    delete[] a;
    delete[] b;

    std::string outfname = infname;
    replace(outfname, ".pgm", "_simple.pgm");

    if (writePGMB_image(outfname.c_str(), b[0], rows, cols, 255) == 0)
        exit(1);

    printf("czas binaryzacji : %f ms\n", elapsed);

    return 0;
}
