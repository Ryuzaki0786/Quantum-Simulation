#include <cstdio>
#include <cmath>
#include <chrono>

// ... applyGateKernel unchanged above ...
__global__ void applyGateKernel(double* re, double* im, int size, int bit,
                                double g00r, double g00i, double g01r, double g01i,
                                double g10r, double g10i, double g11r, double g11i) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= size) return;        // kill the extra threads from ceiling division
    if ((idx & bit) != 0) return;   // only the lower index of each pair works

    int j = idx | bit;              // partner index (target bit flipped on)

    double ar = re[idx], ai = im[idx];   // snapshot a  (old amplitude at idx)
    double br = re[j],   bi = im[j];     // snapshot b  (old amplitude at j)

    // new[idx] = g00*a + g01*b   (complex multiply, written out)
    re[idx] = (g00r*ar - g00i*ai) + (g01r*br - g01i*bi);
    im[idx] = (g00r*ai + g00i*ar) + (g01r*bi + g01i*br);

    // new[j]   = g10*a + g11*b
    re[j]   = (g10r*ar - g10i*ai) + (g11r*br - g11i*bi);
    im[j]   = (g10r*ai + g10i*ar) + (g11r*bi + g11i*br);
}

int main() {
    int n = 20;
    int size = 1 << n;
    size_t bytes = size * sizeof(double);

    double* h_re = (double*)malloc(bytes);
    double* h_im = (double*)malloc(bytes);
    double amp = 1.0 / sqrt((double)size);
    for (int i = 0; i < size; ++i) { h_re[i] = amp; h_im[i] = 0.0; }

    double *d_re, *d_im;
    cudaMalloc(&d_re, bytes);
    cudaMalloc(&d_im, bytes);

    int target = 0, bit = 1 << target;
    double s = 1.0 / sqrt(2.0);
    int threads = 256;
    int blocks = (size + threads - 1) / threads;

    // CUDA events time GPU work precisely (they sit in the GPU's own stream)
    cudaEvent_t t0, t1, t2, t3;
    cudaEventCreate(&t0); cudaEventCreate(&t1);
    cudaEventCreate(&t2); cudaEventCreate(&t3);

    // --- time the transfer IN ---
    cudaEventRecord(t0);
    cudaMemcpy(d_re, h_re, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_im, h_im, bytes, cudaMemcpyHostToDevice);
    cudaEventRecord(t1);

    // --- time the KERNEL only ---
    applyGateKernel<<<blocks, threads>>>(d_re, d_im, size, bit,
                                         s,0.0, s,0.0, s,0.0, -s,0.0);
    cudaEventRecord(t2);

    // --- time the transfer OUT ---
    cudaMemcpy(h_re, d_re, bytes, cudaMemcpyDeviceToHost);
    cudaMemcpy(h_im, d_im, bytes, cudaMemcpyDeviceToHost);
    cudaEventRecord(t3);
    cudaEventSynchronize(t3);         // wait for all recorded work to finish

    float ms_in, ms_kernel, ms_out;
    cudaEventElapsedTime(&ms_in,     t0, t1);
    cudaEventElapsedTime(&ms_kernel, t1, t2);
    cudaEventElapsedTime(&ms_out,    t2, t3);

    // --- oracle again, so we know this run is still correct ---
    double total = 0.0;
    for (int i = 0; i < size; ++i) total += h_re[i]*h_re[i] + h_im[i]*h_im[i];

    printf("total probability = %.10f\n", total);
    printf("transfer in   : %.3f ms\n", ms_in);
    printf("kernel only   : %.3f ms\n", ms_kernel);
    printf("transfer out  : %.3f ms\n", ms_out);
    printf("GPU round-trip: %.3f ms  (in + kernel + out)\n", ms_in + ms_kernel + ms_out);

    cudaFree(d_re); cudaFree(d_im);
    free(h_re); free(h_im);
    return 0;
}
