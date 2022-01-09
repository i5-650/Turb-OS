#include <system/ubsan/ubsan.hpp>
#include <lib/string.hpp>
#include <drivers/display/serial/serial.hpp>

static void printUBsan(const char* message, struct source_location location){
	turbo::serial::log("UBsan: %s\n\t%s file\n\t%d line\n\t%d column\n", message, location.file, location.line, location.column);
}

extern "C" void __ubsan_handle_add_overflow(struct overflow_data *data){
	printUBsan("+ overflow", data->location);
}

extern "C" void __ubsan_handle_sub_overflow(struct overflow_data *data){
	printUBsan("- overflow", data->location);
}

extern "C" void __ubsan_handle_mul_overflow(struct overflow_data *data){
	printUBsan("* overflow", data->location);
}

extern "C" void __ubsan_handle_divrem_overflow(struct overflow_data *data){
	printUBsan("/ overflow", data->location);
}

extern "C" void __ubsan_handle_negate_overflow(struct overflow_data *data){
	printUBsan("! overflow", data->location);
}

extern "C" void __ubsan_handle_pointer_overflow(struct overflow_data *data){
	printUBsan("*ptr overflow", data->location);
}

extern "C" void __ubsan_handle_shift_out_of_bounds(struct shift_out_of_bounds_data *data){
	printUBsan("shift out of bounds", data->location);
}

extern "C" void __ubsan_handle_load_invalid_value(struct invalid_value_data *data){
	printUBsan("invalid load value", data->location);
}

extern "C" void __ubsan_handle_out_of_bounds(struct out_of_bounds_data *data){
	printUBsan("array out of bounds", data->location);
}

extern "C" void __ubsan_handle_type_mismatch_v1(struct type_mismatch_data *data, uintptr_t ptr){
	if (ptr == 0){
		printUBsan("use of NULL pointer", data->location);
	}
	else if (ptr & ((1 << data->logAlignment) - 1)){
		printUBsan("use of misaligned pointer", data->location);
	}
	else {
		printUBsan("no space for object", data->location);
	}
}

extern "C" void __ubsan_handle_vla_bound_not_positive(struct negative_vla_data *data){
	printUBsan("variable-length argument is negative", data->location);
}

extern "C" void __ubsan_handle_nonnull_return(struct nonnull_return_data *data){
	printUBsan("non-null return is null", data->location);
}

extern "C" void __ubsan_handle_nonnull_return_v1(struct nonnull_return_data *data){
	printUBsan("non-null return is null", data->location);
}

extern "C" void __ubsan_handle_nonnull_arg(struct nonnull_arg_data *data){
	printUBsan("non-null argument is null", data->location);
}

extern "C" void __ubsan_handle_builtin_unreachable(struct unreachable_data *data){
	printUBsan("unreachable code reached", data->location);
}

extern "C" void __ubsan_handle_invalid_builtin(struct invalid_builtin_data *data){
	printUBsan("invalid builtin", data->location);
}