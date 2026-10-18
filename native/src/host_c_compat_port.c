/* MinGW spelling referenced by original Wii SDK C sources. */
int __abs(int value) {
    if (value >= 0) return value;
    /* Match two's-complement C runtime behavior for INT_MIN without signed UB. */
    return (int)(0u - (unsigned int)value);
}
