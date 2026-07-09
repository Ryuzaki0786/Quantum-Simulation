#include <cstdio>
#include <cmath>

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
    int n = 20;                       // 20 qubits
    int size = 1 << n;                // 2^20 = 1,048,576 amplitudes

    size_t bytes = size * sizeof(double);

    // --- host arrays: build a uniform superposition ---
    double* h_re = (double*)malloc(bytes);
    double* h_im = (double*)malloc(bytes);
    double amp = 1.0 / sqrt((double)size);   // equal amplitude, so sum|amp|^2 = 1
    for (int i = 0; i < size; ++i) { h_re[i] = amp; h_im[i] = 0.0; }

    // --- 1. allocate device memory ---
    double *d_re, *d_im;
    cudaMalloc(&d_re, bytes);
    cudaMalloc(&d_im, bytes);

    // --- 2. copy host -> device (across PCIe) ---
    cudaMemcpy(d_re, h_re, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_im, h_im, bytes, cudaMemcpyHostToDevice);

    // --- 3. launch: Hadamard on qubit 0 ---
    int target = 0;
    int bit = 1 << target;
    double s = 1.0 / sqrt(2.0);        // Hadamard = (1/sqrt2)[[1,1],[1,-1]]
    int threads = 256;
    int blocks = (size + threads - 1) / threads;   // ceiling division
    applyGateKernel<<<blocks, threads>>>(d_re, d_im, size, bit,
                                         s, 0.0,  s, 0.0,     // g00, g01
                                         s, 0.0, -s, 0.0);    // g10, g11
    cudaDeviceSynchronize();          // wait for the GPU to finish

    // --- 4. copy device -> host ---
    cudaMemcpy(h_re, d_re, bytes, cudaMemcpyDeviceToHost);
    cudaMemcpy(h_im, d_im, bytes, cudaMemcpyDeviceToHost);

    // --- correctness oracle: total probability must be 1 ---
    double total = 0.0;
    for (int i = 0; i < size; ++i)
        total += h_re[i]*h_re[i] + h_im[i]*h_im[i];
    printf("total probability = %.10f\n", total);

    // --- 5. free everything ---
    cudaFree(d_re); cudaFree(d_im);
    free(h_re); free(h_im);
    return 0;
}
