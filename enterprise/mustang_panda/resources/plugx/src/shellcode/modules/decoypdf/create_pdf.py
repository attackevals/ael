
pdf_file = "Meeting Invitation.pdf"

with open(pdf_file, "rb") as f:
    data = f.read()

with open("embedded_pdf.cpp", "w") as cpp:
    cpp.write('#include "embedded_pdf.hpp"\n')
    cpp.write("void load_pdf_data(pdf_data_ctx* ctx) {\n")
    cpp.write("    static const unsigned char pdf_file_data[] = {\n")
    cpp.write("    ")
    cpp.write(", ".join(f"0x{b:02x}" for b in data))
    cpp.write("\n    };\n")
    cpp.write("    ctx->pdf_data = pdf_file_data;\n")
    cpp.write(f"    ctx->pdf_data_len = {len(data)};\n")
    cpp.write("};\n")

with open("embedded_pdf.hpp", "w") as hpp:
    hpp.write("#pragma once\n\n")
    hpp.write("struct pdf_data_ctx {\n")
    hpp.write("    const unsigned char* pdf_data;\n")
    hpp.write("    int pdf_data_len;\n")
    hpp.write("};\n\n")
    hpp.write("void load_pdf_data(pdf_data_ctx* ctx);\n")
