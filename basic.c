#include "svpng.inc"
#include <math.h> // fminf(), sinf(), cosf()
#include <stdlib.h> // rand(), RAND_MAX

#define W 512
#define H 512
#define TWO_PI 6.28318530718f
#define N 64
#define MAX_STEP 10
#define MAX_DIS 2.0f
#define EPSILON 1e-6f

unsigned char img[W * H * 3];

typedef struct { float sd, emissive; } Result;

float circleSDF(float x, float y, float cx, float cy, float r)
{
    float ux = x - cx, uy = y - cy;
    return sqrtf(ux * ux + uy * uy) - r;
}

Result unionOp(Result a, Result b)
{
    return a.sd > b.sd ? b : a;
}


Result sence(float x, float y)
{
#if 1
    Result r1 = { circleSDF(x, y, 0.3f, 0.3f, 0.10f), 2.0f };
    Result r2 = { circleSDF(x, y, 0.3f, 0.7f, 0.05f), 0.8f };
    Result r3 = { circleSDF(x, y, 0.7f, 0.5f, 0.10f), 0.0f };
    return unionOp(unionOp(r1, r2), r3);
    
#else
    Result r1 = { circleSDF(x, y, 0.5f, 0.5f, 0.10f), 2.0f };
    return r1;
#endif
}



float trace(float x, float y, float dx, float dy)
{
    float t = 0.001f;
    for(int i = 0; i < MAX_STEP && t < MAX_DIS;i++)
    {
        Result r = sence(x + dx * t, y + dy * t);
        if(r.sd < EPSILON)
            return r.emissive;
        t += r.sd;
    }

    return 0.0f;
}

float sample(float x, float y)
{
    float sum = 0.0f;
    for (int i = 0; i < N; i++)
    {
        float a = TWO_PI * rand() / RAND_MAX;
        // float a = TWO_PI / N * i;
        // float a = TWO_PI / N * (i + (float)rand() / RAND_MAX) ;
        sum += trace(x, y, cosf(a), sinf(a));
    }

    return sum / N ;
}

int main()
{
    unsigned char* p = img;
    for(int y = 0; y< H; y++)
        for(int x = 0;  x< W; x++, p+=3)
            p[0] = p[1] = p[2] = (int)fminf(sample((float)x / W, (float)y / H) * 255.0f, 255.0f);

    svpng(fopen("L_3_random_sampling_trace_max_step_10.png", "wb"), W, H, img, 0);
    return 0;
}