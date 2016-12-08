#include <string.h>

// Define a struct, Pixel, that contains RGB values as characters
typedef struct pixel {
  unsigned char r, g, b;
} pixel;

// Function Prototypes
void write3(FILE *out, pixel *buffer, int *width, int *height, int *maxColor, int *ver);
void write6(FILE *out, pixel *buffer, int *width, int *height, int *maxColor, int *ver);
void read3(FILE *in, pixel *buffer, int *width, int *height, int *maxColor);
void read6(FILE *in, pixel *buffer, int *width, int *height, int *maxColor);
int initiate(FILE *fp, int *width, int *height, int *maxColor, int *ver);
void fill(pixel *buffer, int *width, int *height);
double clamp(double value);


void fill(pixel *buffer, int *width, int *height) {
  int i = 0;
  int length = (*width) * (*height);
  while (i < length) {
    buffer[i].r = 0;
    buffer[i].g = 0;
    buffer[i].b = 0;
    i++;
  }
}

int initiate(FILE *in, int *width, int *height, int *maxColor, int *ver)
{
  // File does not exist
  if (!in) {
    return 2;
  }
  char c; // Character pointer to grab each character

  // Validate the Magic Number
  fscanf(in, "P%c\n", &c);
  if (c != '6') {
    if (c != '3') {
      return 1;
    }
  }

  *ver = atoi(&c);

  c = getc(in);

  // Skip Comments in the Header
  while (c == '#') {
    do {
      c = getc(in);
    }
    while (c != '\n');
    c = getc(in);
  }

  // Check that the next char is a digit
  if (!isdigit(c)) {
    return 3;
  }

  // Return the digit
  ungetc(c, in);

  // Scan for the maximum width, height, and maximum color spectrum
  fscanf(in, "%d%d%d\n", width, height, maxColor);

  if (*maxColor > 255) {
    printf("\nMaximum color value supplied has exceeed"
           " supported range. Constraining to 255.");
  }

  return 0;
}

void read3(FILE *in, pixel *buffer, int *width, int *height, int *maxColor) {
  int c, r, g, b;
  int i = 0;
  int length = (*width) * (*height);

  while (i < length) {
      if ((c = fgetc(in)) != EOF) {
        // Return the character
        ungetc(c, in);
        // Grab the next three integers and assign them to the pixel buffer
        if (fscanf(in, "%d%d%d", &r, &g, &b) == 3) {
          buffer[i].r = r;
          buffer[i].g = g;
          buffer[i].b = b;
        }
      i++;
    }
  }
}

void read6(FILE *in, pixel *buffer, int *width, int *height, int *maxColor) {
  unsigned char *inBuf = malloc(sizeof(unsigned char));
  int i = 0;
  int conv;
  int length = (*width) * (*height);
  // while there are bytes to be read in the file
  while (fread(inBuf, 1, 1, in) && i < length) {
    // Get each pixel into the char buffer and add to the pixel buffer
    conv = *inBuf;
    buffer[i].r = conv;
    fread(inBuf, 1, 1, in);
    conv = *inBuf;
    buffer[i].g = conv;
    fread(inBuf, 1, 1, in);
    conv = *inBuf;
    buffer[i].b = conv;
    i++;
  }
}

void write3(FILE *out, pixel *buffer, int *width, int *height, int *maxColor, int *ver) {
  int i = 0, lineLen = 1;
  int length = (*width) * (*height);
  fprintf(out, "P%d \n#Converted by Gage Myers\n%d %d %d\n", *ver, *width, *height, *maxColor);
  while (i < length) {
      if ((35 % lineLen) == 35) {
        fprintf(out, "\n");
        lineLen = 1;
      }
      // Get each pixel and write out its components in their ascii representation
      fprintf(out, "%d ", buffer[i].r);
      fprintf(out, "%d ", buffer[i].g);
      fprintf(out, "%d ", buffer[i].b);
      lineLen++;
      i++;
    }
}


void write6(FILE *out, pixel *buffer, int *width, int *height, int *maxColor, int *ver) {
  int i = 0;
  int length = (*width) * (*height);
  // Write the header
  fprintf(out, "P%d \n#Converted by Gage Myers\n %d %d %d\n", *ver, *width, *height, *maxColor);
  // Write out each pixel to the file
  while (i < length) {
    fwrite(&(buffer[(i)].r), sizeof(unsigned char), 1, out);
    fwrite(&(buffer[(i)].g), sizeof(unsigned char), 1, out);
    fwrite(&(buffer[(i)].b), sizeof(unsigned char), 1, out);
    i++;
  }
}

double clamp(double value) {
  if (value > 1) {
    return 1;
  } else if (value < 0) {
    return 0;
  } else {
    return value;
  }
}
