#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double* normalize_vector(int vdim, double* vp) {
    double* n_vp;

    int sum = 0;
    for(int i=0; i<vdim; i++) {
        sum += (vp[i] * vp[i]);
    }

    if(sum == 0) {
        return NULL;
    }

    double weight = sqrt(sum);

    n_vp = malloc(sizeof(double) * vdim);
    for(int i=0; i<vdim; i++) {
        n_vp[i] = vp[i] / weight;
    }

    return n_vp;
}

double euclidean_distance(int vdim, double* vp1, double* vp2) {
    double sum = 0;
    for(int i=0; i<vdim; i++) {
        sum += pow(vp2[i] - vp1[i],2);
    }
    return sqrt(sum);
}

double inner_product(int vdim, double* vp1, double* vp2) {
    double product = 0.0;

    for(int i=0; i < vdim; i++) {
        product += vp1[i] * vp2[i];
    }

    return product;
}

double dot_product(int vdim, double* vp1, double* vp2) {
    double sum = 0;
    for(int i=0; i<vdim; i++) {
        sum += pow(vp2[i] - vp1[i],2);
    }
    return sqrt(sum);
}

double magnitude(int vdim, double* vp) {
    double magnitude = 0.0;

    for(int i=0; i < vdim; i++) {
        magnitude += vp[i] * vp[i];
    }

    return sqrt(magnitude);
}

double cosine_similarity(int vdim, double* vp1, double* vp2) {
    double product = inner_product(vdim, vp1, vp2);
    double magnitude_1 =  magnitude(vdim, vp1);
    double magnitude_2 =  magnitude(vdim, vp2);
    
    if(magnitude_1 == 0 || magnitude_2 == 0) {
        return 0;
    }

    return product / (magnitude_1 * magnitude_2);
}   

double manhattan_distance(int vdim, double* vp1, double* vp2) {
    double distance = 0.0;

    for(int i=0; i<vdim; i++) {
        distance += fabs(vp1[i] - vp2[i]);
    }

    return distance;
}   

double minskowski_distance(int vdim, double* vp1, double* vp2, double p_value) {
    double distance = 0.0;

    if(p_value == 0)
        return distance;

    for(int i=0; i<vdim; i++) {
        distance += pow(fabs(vp1[i] - vp2[i]), p_value);
    }

    return pow(distance, 1.0 / p_value);
}   

