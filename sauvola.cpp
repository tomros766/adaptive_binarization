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

void sauvola_binarization(unsigned char **img, unsigned char **output, int width, int height, float threshold, int radius)
{
    int xx, yy, i, j, x_offset, y_offset;
    int sum, count;
    float mean, variance, std_dev, local_threshold;

    count =  (2 * radius + 1) * (2 * radius + 1);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            sum = 0;

            // warunki brzegowe
            x_offset = 0;
            y_offset = 0;
        
            if (x - radius < 0) {
                x_offset = radius - x + 1;
            }
            else if (x + radius >= width) {
                x_offset = width - x - radius - 1;
            }

            if (y - radius < 0) {
                y_offset = radius - y + 1;
            }
            else if (y + radius >= height) {
                y_offset = height - y - radius - 1;
            }

            // obliczenie sumy
            for (i = -radius; i <= radius; i++)
            {
                for (j = -radius; j <= radius; j++)
                {
                    xx = x + j + x_offset;
                    yy = y + i + y_offset;
                    
                    sum += img[yy][xx];
                }
            }

            mean = (float)sum / count;

            // obliczenie sumy kwadratow do odchylenia standardowego
            sum = 0;
            for (i = -radius; i <= radius; i++)
            {
                for (j = -radius; j <= radius; j++)
                {
                    xx = x + j + x_offset;
                    yy = y + i + y_offset;
                    
                    sum += pow(img[yy][xx] - mean, 2);
                }
            }
            variance = (float)sum / count;
            std_dev = sqrt(variance);


            local_threshold = mean * (1 + threshold * (std_dev / 128 - 1));
            output[y][x] = img[y][x] < local_threshold ? 0 : 255;
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
            sum += square ? img[y][x] * img[y][x] : img[y][x];
            
            integral[y][x] = y == 0 ? sum : integral[y - 1][x] + sum;
        }
    }
}

void check_integral(unsigned char **img, unsigned int **integral, int x1, int x2, int y1, int y2, bool square)
{
    int i, j;
    int img_val;
    for (i = y1; i < y2; i++)
    {
        for (j = x1; j < x2; j++)
        {
            img_val = square ? img[i][j] * img[i][j] : img[i][j];
            if (integral[i][j] != img_val + integral[i - 1][j] + integral[i][j - 1] - integral[i - 1][j - 1])
            {
                printf("ERROR: Integral image calculated wrongly!\n");
                return;
            }
        }
    }
    printf("Integral image calculated correctly!\n");
}

void sauvola_binarization_integral(unsigned char **img, unsigned char **output, unsigned int **integral, unsigned int **integral_sq, int width, int height, float threshold, int radius)
{
    float local_threshold, mean, variance, std_dev;
    int x1, x2, y1, y2, count, sum, sum_sq, x_offset, y_offset;

    count =  (2 * radius + 1) * (2 * radius + 1);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            x1 = x - radius;
            y1 = y - radius;
            x2 = x + radius;
            y2 = y + radius;

            // Warunki brzegowe
            x_offset = 0;
            y_offset = 0;
            if (x1 < 0) x_offset = radius - x1 + 1;
            if (y1 < 0) y_offset = radius - y1 + 1;
            if (x2 >= width) x_offset = width - x2 - radius - 1;
            if (y2 >= height) y_offset = height - y2 - radius - 1;

            x1 += x_offset;
            x2 += x_offset;
            y1 += y_offset;
            y2 += y_offset;

            // Suma z integral image
            sum = integral[y2][x2];
            sum_sq = integral_sq[y2][x2];

            if (x1 > 0) {
                sum -= integral[y2][x1 -1];
                sum_sq -= integral_sq[y2][x1 -1];
            }
            if (y1 > 0) {
                sum -= integral[y1 - 1][x2];
                sum_sq -= integral_sq[y1 - 1][x2];
            }
            if (x1 > 0 && y1 > 0) {
                sum += integral[y1 - 1][x1 - 1];
                sum_sq += integral_sq[y1 - 1][x1 - 1];
            }

            mean = (float)sum / count;
            variance = (float)sum_sq / count - mean * mean;
            std_dev = sqrt(variance);

            local_threshold = mean * (1 + threshold * (std_dev / 128 - 1));
            output[y][x] = img[y][x] < local_threshold ? 0 : 255;
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

    // Tablica wejsciowa a, wyjsciowa b i tablice na integral image sum i sum kwadratow
    unsigned char **a = new unsigned char *[height];
    a[0] = new unsigned char[height * width];
    for (int i = 1; i < height; i++) a[i] = a[i - 1] + width;

    unsigned char **b = new unsigned char *[height];
    b[0] = new unsigned char[height * width];
    for (int i = 1; i < height; i++) b[i] = b[i - 1] + width;

    unsigned int **integral = new unsigned int *[height];
    integral[0] = new unsigned int[height * width];
    for (int i = 1; i < height; i++) integral[i] = integral[i - 1] + width;

    unsigned int **integral_sq = new unsigned int *[height];
    integral_sq[0] = new unsigned int[height * width];
    for (int i = 1; i < height; i++) integral_sq[i] = integral_sq[i - 1] + width;

    if (readPGMB_data(a[0], infname.c_str(), hpos, height, width, max_color) == 0) exit(1);

    tt.Begin(); // start to measure the time

    if (with_integral)
    {
        integral_image(a, integral, width, height, false);
        integral_image(a, integral_sq, width, height, true);
        sauvola_binarization_integral(a, b, integral, integral_sq, width, height, 0.03, 13);
    }
    else
    {
        sauvola_binarization(a, b, width, height, 0.03, 13);
    }

    double elapsed = tt.End(); // stop and read elapsed time in ms (miliseconds)

    if (with_integral) {
        check_integral(a, integral, 1, width, 1, height, false);
        check_integral(a, integral_sq, 1, width, 1, height, true);
    }

    std::string outfname = infname;
    const char* suffix = with_integral ? "_int_sauvola.pgm" : "_sauvola.pgm";
    replace(outfname, ".pgm", suffix);

    if (writePGMB_image(outfname.c_str(), b[0], height, width, 255) == 0) exit(1);
    
    delete[] a;
    delete[] b;
    delete[] integral;
    delete[] integral_sq;

    const char *integral_msg = with_integral ? "z uzyciem integral image" : "";
    printf("czas binaryzacji %s: %f ms\n", integral_msg, elapsed);

    return 0;
}
