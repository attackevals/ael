#pragma once

struct pdf_data_ctx {
    const unsigned char* pdf_data;
    int pdf_data_len;
};

void load_pdf_data(pdf_data_ctx* ctx);
