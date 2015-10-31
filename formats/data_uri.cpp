#include "data_uri.h"

#include <algorithm>
#include <iostream>
#include <string>

#include "base64.h"

using std::cout;
using std::endl;
using std::string;

size_t DataURI::Leanify(size_t size_leanified /*= 0*/)
{
    uint8_t *p_read = fp, *p_write = fp - size_leanified;

    while (p_read < fp + size)
    {
        const string magic = "data:image/";
        uint8_t *new_p_read = std::search(p_read, fp + size, magic.begin(), magic.end());
        // move anything in between
        memmove(p_write, p_read, new_p_read - p_read);
        p_write += new_p_read - p_read;
        p_read = new_p_read;

        if (p_read >= fp + size)
        {
            break;
        }

        const string base64_sign = ";base64,";
        // data:image/png;base64,   data:image/jpeg;base64,
        // maximum gap is 4
        uint8_t *search_end = p_read + magic.size() + 4 + base64_sign.size();
        uint8_t *start = std::search(p_read + magic.size(), search_end, base64_sign.begin(), base64_sign.end()) + base64_sign.size();

        memmove(p_write, p_read, start - p_read);
        p_write += start - p_read;
        p_read = start;

        if (p_read > search_end)
        {
            continue;
        }

        const char quote[] = { '\'', '"', ')' };
        uint8_t *end = std::find_first_of(p_read, fp + size, quote, quote + sizeof(quote));
        if (end < fp + size)
        {
            if (is_verbose)
            {
                cout << string(reinterpret_cast<char *>(new_p_read), start + 8 - new_p_read) << "... found at offset 0x" << std::hex << new_p_read - fp << endl;
            }
            size_t new_size = Base64(p_read, end - p_read).Leanify(p_read - p_write);
            p_write += new_size;
            p_read = end;
        }
        else
        {
            memmove(p_write, p_read, fp + size - p_read);
            p_write += fp + size - p_read;
            p_read = fp + size;
        }
    }
    fp -= size_leanified;
    size = p_write - fp;
    return size;
}