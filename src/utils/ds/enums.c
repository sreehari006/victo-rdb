#include <string.h>
#include "../../commons/constants.h"

int to_int_logical_ops(const char* op) {
    if(strcasecmp(op, "<=") == 0) {
        return -2;
    } else if(strcasecmp(op, "<") == 0) {
        return -1;
    } else if(strcasecmp(op, "=") == 0) {
        return 0;
    } else if(strcasecmp(op, ">") == 0) {
        return 1;
    } else if(strcasecmp(op, ">=") == 0) {
        return 2;
    } 

    return 0;
}


int to_int_vd_method(const char* op) {
    if(strcasecmp(op, "EUCLIDEAN_DISTANCE") == 0) {
        return 0;
    } else if(strcasecmp(op, "COSINE_SIMILARITY") == 0) {
        return 1;
    } else if(strcasecmp(op, "MANHATTAN_DISTANCE") == 0) {
        return 2;
    } else if(strcasecmp(op, "MINKOWSKI_DISTANCE") == 0) {
        return 3;
    } else if(strcasecmp(op, "DOT_PRODUCT") == 0) {
        return 4;
    } 

    return 0;
}

const char* to_string_logical_ops(const int op) {
    const char* result;

    if(op == -2) {
        result = "<=";
    } else if(op == -1) {
        result = "<";
    } else if(op == 0) {
        result = "=";
    } else if(op == 1) {
        result = ">";
    } else if(op == 2) {
        result = ">=";
    } else {
        result = "UN_KNOWN";
    }

    return result;
}


const char* to_string_vd_method(const int op) {
    const char* result;

    if(op == 0) {
        result = "EUCLIDEAN_DISTANCE";
    } else if(op == 1) {
        result = "COSINE_SIMILARITY";
    } else if(op == 2) {
        result = "MANHATTAN_DISTANCE";
    } else if(op == 3) {
        result = "MINKOWSKI_DISTANCE";
    } else if(op == 4) {
        result = "DOT_PRODUCT";
    } else {
        result = "UN_KNOWN";
    }

    return result;
}