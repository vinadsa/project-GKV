#include <assert.h>
#include <fstream>
#include "imageloader.h"

using namespace std;

Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h) {}

Image::~Image() {
    delete[] pixels;
}

namespace {
    //Konversi 4 buah karakter ke integer,
    //menggunakan bentuk little-endian
    int toInt(const char* bytes) {
        return (int)(((unsigned char)bytes[3] << 24) |
                     ((unsigned char)bytes[2] << 16) |
                     ((unsigned char)bytes[1] << 8) |
                     (unsigned char)bytes[0]);
    }

    //Konversi 2 buah karakter ke integer,
    //menggunakan bentuk little-endian
    short toShort(const char* bytes) {
        return (short)(((unsigned char)bytes[1] << 8) |
                       (unsigned char)bytes[0]);
    }

    //Membaca 4 byte selanjutnya sebagai integer,
    //menggunakan bentuk little-endian
    int readInt(ifstream &input) {
        char buffer[4];
        input.read(buffer, 4);
        return toInt(buffer);
    }

    //Membaca 2 byte selanjutnya sebagai short,
    //menggunakan bentuk little-endian
    short readShort(ifstream &input) {
        char buffer[2];
        input.read(buffer, 2);
        return toShort(buffer);
    }

    template<class T>
    class auto_array {
        private:
            T* array;
            mutable bool isReleased;
        public:
            explicit auto_array(T* array_ = NULL) :
                array(array_), isReleased(false) {
            }

            auto_array(const auto_array<T> &aarray) {
                array = aarray.array;
                isReleased = aarray.isReleased;
                aarray.isReleased = true;
            }

            ~auto_array() {
                if (!isReleased && array != NULL) {
                    delete[] array;
                }
            }

            T* get() const {
                return array;
            }

            T &operator*() const {
                return *array;
            }

            void operator=(const auto_array<T> &aarray) {
                if (!isReleased && array != NULL) {
                    delete[] array;
                }
                array = aarray.array;
                isReleased = aarray.isReleased;
                aarray.isReleased = true;
            }

            T* operator->() const {
                return array;
            }

            T* release() {
                isReleased = true;
                return array;
            }

            void reset(T* array_ = NULL) {
                if (!isReleased && array != NULL) {
                    delete[] array;
                }
                array = array_;
            }

            T* operator+(int i) {
                return array + i;
            }

            T &operator[](int i) {
                return array[i];
            }
    };
}

Image* loadBMP(const char* filename) {
    ifstream input;
    input.open(filename, ifstream::binary);
    assert(!input.fail() || !"File tidak ditemukan!!!"); // [cite: 22]
    char buffer[2];
    input.read(buffer, 2);
    assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Bukan file bitmap!!!"); // [cite: 23]
    input.ignore(8);
    int dataOffset = readInt(input);

    //Baca header
    int headerSize = readInt(input);
    int width;
    int height;
    switch (headerSize) {
        case 40:
            //V3
            width = readInt(input);
            height = readInt(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Gambar tidak 24 bits per pixel!"); // [cite: 24]
            assert(readShort(input) == 0 || !"Gambar dikompres!"); // [cite: 25]
            break;
        case 12:
            //OS/2 V1
            width = readShort(input);
            height = readShort(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Gambar tidak 24 bits per pixel!"); // [cite: 26]
            break;
        case 64:
            //OS/2 V2
            assert(!"Tidak dapat mengambil OS/2 V2 bitmaps"); // [cite: 27]
            break;
        case 108:
            //Windows V4
            assert(!"Tidak dapat mengambil Windows V4 bitmaps"); // [cite: 27]
            break;
        case 124:
            //Windows V5
            assert(!"Tidak dapat mengambil Windows V5 bitmaps"); // [cite: 28]
            break;
        default:
            assert(!"Format bitmap ini tidak diketahui!"); // [cite: 28]
    }

    //Membaca data
    int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4); // [cite: 29]
    int size = bytesPerRow * height; // [cite: 29]
    auto_array<char> pixels(new char[size]); // [cite: 29]
    input.seekg(dataOffset, ios_base::beg); // [cite: 30]
    input.read(pixels.get(), size); // [cite: 30]

    //Mengambil data yang mempunyai format benar
    auto_array<char> pixels2(new char[width * height * 3]); // [cite: 30]
    for(int y = 0; y < height; y++) { // [cite: 31]
        for(int x = 0; x < width; x++) { // [cite: 31]
            for(int c = 0; c < 3; c++) { // [cite: 31]
                pixels2[3 * (width * y + x) + c] = // [cite: 31]
                    pixels[bytesPerRow * y + 3 * x + (2 - c)]; // [cite: 31]
            }
        }
    }

    input.close();
    return new Image(pixels2.release(), width, height); // [cite: 31]
}