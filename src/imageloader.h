/* Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. // [cite: 32]
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. // [cite: 33]
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. // [cite: 34]
 */

/* File for "Putting It All Together" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */ // [cite: 35]

#ifndef IMAGE_LOADER_H_INCLUDED
#define IMAGE_LOADER_H_INCLUDED

//Represents an image
class Image {
    public:
        Image(char* ps, int w, int h); // [cite: 35]
        ~Image(); // [cite: 36]

        /* An array of the form (R1, G1, B1, R2, G2, B2, ...) indicating the
         * color of each pixel in image. Color components range from 0 to 255. // [cite: 36, 37]
         * The array starts the bottom-left pixel, then moves right to the end
         * of the row, then moves up to the next row, and so on. This is the // [cite: 37]
         * format in which OpenGL likes images. // [cite: 38]
         */
        char* pixels;
        int width;
        int height;
};

//Reads a bitmap image from file.
Image* loadBMP(const char* filename); // [cite: 39]

#endif