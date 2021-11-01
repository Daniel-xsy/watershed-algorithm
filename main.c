#include"./include/main.h"


int main()
{
    unsigned char *pixels;
    unsigned int width, height;
    unsigned int error;
    
    error = loadgreybmp_decode_file("img/ex.bmp", &pixels, &width, &height);
    printf("error: %i\n",error);

    //gauss_filter(&pixels, width, height, 11, 5);
    //median_filter(&pixels, width, height, 11);

    if(watershed(&pixels, width, height, 255) == 0)
    {
        printf("error in watershed\n");
        return 0;
    }

    error = loadgreybmp_encode_file("img/ws.bmp", pixels, width, height);
    printf("error: %i\n",error);

    free(pixels);
    return 1;
}
