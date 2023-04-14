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

void niblack_binarization(unsigned char **img, unsigned char **output, int width, int height, float threshold, int radius)
{
    int x, y, xx, yy, i, j;
    int sum, count;
    float mean, variance, std_dev;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            sum = 0;
            count = 0;
            for (i = -radius; i <= radius; i++)
            {
                for (j = -radius; j <= radius; j++)
                {
                    xx = x + j;
                    yy = y + i;
                    if (xx >= 0 && xx < width && yy >= 0 && yy < height)
                    {
                        sum += img[yy][xx];
                        count++;
                    }
                }
            }

            mean = (float)sum / count;
            sum = 0;
            for (j = -radius; j <= radius; j++)
            {
                for (i = -radius; i <= radius; i++)
                {
                    if (x + i >= 0 && x + i < width && y + j >= 0 && y + j < height)
                    {
                        sum += pow(img[(y + j)][(x + i)] - mean, 2);
                    }
                }
            }
            variance = (float)sum / count;
            std_dev = sqrt(variance);

            if (img[y][x] < mean - threshold * std_dev)
            {
                output[y][x] = 0;
            }
            else
            {
                output[y][x] = 255;
            }
        }
    }
}

void integral_image(unsigned char **img, unsigned int **integral, int width, int height, bool square)
{
    unsigned int sum = 0;
    unsigned int val = 0;
    int x, y;
    for (y = 0; y < height; y++)
    {
        sum = 0;
        for (x = 0; x < width; x++)
        {
            sum += img[y][x];
            if (y == 0)
            {
                val = sum;
            }
            else
            {
                val = integral[y - 1][x] + sum;
            }
            integral[y][x] = square ? val * val : val;
        }
    }
}

void check_integral(unsigned char **img, unsigned int **integral, int x1, int x2, int y1, int y2)
{
    int i, j;

    for (i = y1; i < y2; i++)
    {
        for (j = x1; j < x2; j++)
        {
            if (integral[i][j] != img[i][j] + integral[i - 1][j] + integral[i][j - 1] - integral[i - 1][j - 1])
            {
                printf("ERROR: Integral image calculated wrongly!\n");
            }
        }
    }
    printf("Integral image calculated correctly!\n");
}

void niblack_binarization_integral(unsigned char **img, unsigned char **output, unsigned int **integral, unsigned int **integral_sq, int width, int height, float threshold, int radius)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int x1 = x - radius;
            int y1 = y - radius;
            int x2 = x + radius;
            int y2 = y + radius;
            if (x1 < 0)
                x1 = 0;
            if (y1 < 0)
                y1 = 0;
            if (x2 >= width)
                x2 = width - 1;
            if (y2 >= height)
                y2 = height - 1;

            int count = (x2 - x1 + 1) * (y2 - y1 + 1);

            int sum = integral[y2][x2] - integral[y1][x2] - integral[y2][x1] + integral[y1][x1];
            int sum_sq = integral_sq[y2][x2] - integral_sq[y1][x2] - integral_sq[y2][x1] + integral_sq[y1][x1];
            float mean = (float)sum / count;
            float variance = (float)sum_sq / count - mean * mean;
            float stddev = sqrt(variance);

            if (img[y][x] < mean - threshold * stddev)
            {
                output[y][x] = 0;
            }
            else
            {
                output[y][x] = 255;
            }
        }
    }
}

int main(int argc, char **argv)
{
    int height, width;
    int max_color;
    int hpos, i, j;

    bool with_integral = false;
    if (argc > 1 && strcmp(argv[1], "integral") == 0)
        with_integral = true;

    std::string infname = "img/016_lanczos.pgm";

    if ((hpos = readPGMB_header(infname.c_str(), &height, &width, &max_color)) <= 0)
        exit(1);

    unsigned char **a = new unsigned char *[height];

    a[0] = new unsigned char[height * width];

    for (int i = 1; i < height; i++)
    {
        a[i] = a[i - 1] + width;
    }

    if (readPGMB_data(a[0], infname.c_str(), hpos, height, width, max_color) == 0)
        exit(1);

    // przygotowanie czarno-bialej tablicy wyjsciowej
    // analogicznie jak dla tablicy wejsciowej
    unsigned char **b = new unsigned char *[height];
    b[0] = new unsigned char[height * width];

    for (int i = 1; i < height; i++)
    {
        b[i] = b[i - 1] + width;
    }

    unsigned int **integral = new unsigned int *[height];
    integral[0] = new unsigned int[height * width];

    for (int i = 1; i < height; i++)
    {
        integral[i] = integral[i - 1] + width;
    }

    unsigned int **integral_sq = new unsigned int *[height];
    integral_sq[0] = new unsigned int[height * width];

    for (int i = 1; i < height; i++)
    {
        integral_sq[i] = integral_sq[i - 1] + width;
    }

    tt.Begin(); // start to measure the time

    if (with_integral)
    {
        integral_image(a, integral, width, height, false);
        integral_image(a, integral_sq, width, height, true);
        niblack_binarization_integral(a, b, integral, integral_sq, width, height, 0, 20);
    }
    else
    {
        niblack_binarization(a, b, width, height, 0, 20);
    }

    double elapsed = tt.End(); // stop and read elapsed time in ms (miliseconds)

    if (with_integral)
        check_integral(a, integral, 10, 100, 10, 100);

    std::string outfname = infname;
    const char* suffix = with_integral ? "_int_niblack.pgm" : "_niblack.pgm";
    replace(outfname, ".pgm", suffix);

    if (writePGMB_image(outfname.c_str(), b[0], height, width, 255) == 0)
        exit(1);
    delete[] a;
    delete[] b;

    const char *integral_msg = with_integral ? "z uzyciem integral image" : "";
    printf("czas binaryzacji %s: %f ms\n", integral_msg, elapsed);

    return 0;
}
