.pushsection command_data, "ax", %progbits
.incbin "data/vcchi.bin"
.popsection

.pushsection license_data, "ax", %progbits
.incbin "data/CybLicense.bin"
.popsection
