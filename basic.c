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

float planeSDF(float x, float y, float sx, float sy, float ex, float ey)
{
    // 向量[x - sx,y - sy] 和 [ex, ey] 的点积 
    return (x - sx) * ex + (y - sy) * ey;
}

float segmentSDF(float x, float y, float ax, float ay, float bx, float by)
{
    float ux = bx - ax, uy = by - ay, vx = x - ax, vy = y - ay;
    float t = fmaxf(fminf((ux * vx + uy * vy) / (ux * ux + uy * uy), 1.0f), 0.0f);
    float dx = vx - ux * t, dy = vy - uy * t;
    return sqrtf(dx * dx + dy * dy);
}

float capsuleSDF(float x, float y, float ax, float ay, float bx, float by, float r)
{
    return segmentSDF(x, y, ax, ay, bx, by) - r;
}

float boxSDF(float x, float y, float cx, float cy, float theta, float sx, float sy) {
    float costheta = cosf(theta), sintheta = sinf(theta);
    float dx = fabs((x - cx) * costheta + (y - cy) * sintheta) - sx;
    float dy = fabs((y - cy) * costheta - (x - cx) * sintheta) - sy;
    float ax = fmaxf(dx, 0.0f), ay = fmaxf(dy, 0.0f);
    return fminf(fmaxf(dx, dy), 0.0f) + sqrtf(ax * ax + ay * ay);
}

float triangleSDF(float x, float y, float ax, float ay, float bx, float by, float cx, float cy) {
    float d = fminf(fminf(
        segmentSDF(x, y, ax, ay, bx, by),
        segmentSDF(x, y, bx, by, cx, cy)),
        segmentSDF(x, y, cx, cy, ax, ay));
    return (bx - ax) * (y - ay) > (by - ay) * (x - ax) && 
           (cx - bx) * (y - by) > (cy - by) * (x - bx) && 
           (ax - cx) * (y - cy) > (ay - cy) * (x - cx) ? -d : d;
}

Result unionOp(Result a, Result b)
{
    return a.sd > b.sd ? b : a;
}

Result intersectOp(Result a, Result b) {
    Result r = a.sd > b.sd ? b : a;
    r.sd = a.sd > b.sd ? a.sd : b.sd;
    return r;
}

Result subtractOp(Result a, Result b) {
    Result r = a;
    r.sd = (a.sd > -b.sd) ? a.sd : -b.sd;
    return r;
}

Result sence(float x, float y)
{
#if 0
    Result r1 = { circleSDF(x, y, 0.3f, 0.3f, 0.10f), 2.0f };
    Result r2 = { circleSDF(x, y, 0.3f, 0.7f, 0.05f), 0.8f };
    Result r3 = { circleSDF(x, y, 0.7f, 0.5f, 0.10f), 0.0f };
    return unionOp(unionOp(r1, r2), r3);
#elif 0
    Result a = { circleSDF(x, y, 0.5f, 0.5f, 0.2f), 1.0f };
    Result b = { planeSDF(x, y, 0.0f, 0.5f, 0.0f, 1.0f), 0.8f };
    return intersectOp(a, b);
#elif 0
    Result c = {  capsuleSDF(x, y, 0.4f, 0.4f, 0.6f, 0.6f, 0.1f), 1.0f };
    return c;
#elif 0
    Result d = { boxSDF(x, y, 0.5f, 0.5f, TWO_PI / 16.0f, 0.3f, 0.1f), 1.0f };
    return d;
#elif 1
    Result f = { triangleSDF(x, y, 0.5f, 0.2f, 0.8f, 0.8f, 0.3f, 0.6f), 1.0f };
    return f;
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
        // float a = TWO_PI * rand() / RAND_MAX;
        // float a = TWO_PI / N * i;
        float a = TWO_PI / N * (i + (float)rand() / RAND_MAX) ;
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

    svpng(fopen("Triangle_random_sampling_trace_max_step_10.png", "wb"), W, H, img, 0);
    return 0;
}