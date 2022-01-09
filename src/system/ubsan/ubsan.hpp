#pragma once 

#include <stdint.h>

// https://github.com/OP-TEE/optee_os/blob/master/core/kernel/ubsan.c

struct source_location {
    const char* file;
    uint32_t line;
    uint32_t column;
};

struct ubsan_type_descriptor {
    uint16_t type_kind;
    uint16_t type_info;
    char type_name[];
};

struct overflow_data {
    struct source_location location;
    struct ubsan_type_descriptor *leftType;
    struct ubsan_type_descriptor *rightType;
};

struct shift_out_of_bounds_data {
    struct source_location location;
    struct ubsan_type_descriptor* leftType;
    struct ubsan_type_descriptor* rightType;
};

struct out_of_bounds_data {
    struct source_location location;
    struct ubsan_type_descriptor* arrayType;
    struct ubsan_type_descriptor* indexType;
};

struct invalid_value_data {
    struct source_location location;
    struct ubsan_type_descriptor* type;
};

struct type_mismatch_data {
    struct source_location location;
    struct ubsan_type_descriptor* type;
    uintptr_t logAlignment;
    uint8_t typeCheckKind;
};

struct negative_vla_data{
    struct source_location location;
    struct ubsan_type_descriptor *type;
};

struct nonnull_return_data{
    struct source_location location;
};

struct nonnull_arg_data {
    struct source_location location;
};

struct unreachable_data{
    struct source_location location;
};

struct invalid_builtin_data{
    struct source_location location;
};