#include <mapnik/webp_io.hpp>

namespace mapnik {
    std::string MAPNIK_DECL webp_encoding_error(WebPEncodingError error) {
    std::string os;
    switch (error)
    {
        case VP8_ENC_ERROR_OUT_OF_MEMORY: os = "memory error allocating objects"; break;
        case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY: os = "memory error while flushing bits"; break;
        case VP8_ENC_ERROR_NULL_PARAMETER: os = "a pointer parameter is NULL"; break;
        case VP8_ENC_ERROR_INVALID_CONFIGURATION: os = "configuration is invalid"; break;
        case VP8_ENC_ERROR_BAD_DIMENSION: os = "picture has invalid width/height"; break;
        case VP8_ENC_ERROR_PARTITION0_OVERFLOW: os = "partition is bigger than 512k"; break;
        case VP8_ENC_ERROR_PARTITION_OVERFLOW: os = "partition is bigger than 16M"; break;
        case VP8_ENC_ERROR_BAD_WRITE: os = "error while flushing bytes"; break;
        case VP8_ENC_ERROR_FILE_TOO_BIG: os = "file is bigger than 4G"; break;
        default:
            mapnik::util::to_string(os,error);
            os = "unknown error (" + os + ")"; break;
    }
    return os;
}

}
