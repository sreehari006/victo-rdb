double* normalize_vector(int vdim, double* vp);
double euclidean_distance(int vdim, double* vp1, double* vp2);
double cosine_similarity(int vdim, double* vp1, double* vp2);
double manhattan_distance(int vdim, double* vp1, double* vp2);
double minskowski_distance(int vdim, double* vp1, double* vp2, double p_value);
double dot_product(int vdim, double* vp1, double* vp2);