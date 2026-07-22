#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include "psf.h"

#define PSF_HEADER_SIZE       16u
#define PSX_EXE_HEADER_SIZE   0x800u
#define PSF1_MAX_PROGRAM_SIZE (PSX_EXE_HEADER_SIZE + 2033664u)
#define PSF1_MAX_DEPTH        10

typedef struct {
  uint8_t* program;
  size_t   program_size;
  char*    tags;
} PsfFile;

static uint32_t read_le32(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static void write_le32(uint8_t* p, uint32_t value) {
  p[0] = (uint8_t)value;
  p[1] = (uint8_t)(value >> 8);
  p[2] = (uint8_t)(value >> 16);
  p[3] = (uint8_t)(value >> 24);
}

static void psf_file_free(PsfFile* file) {
  free(file->program);
  free(file->tags);
  memset(file, 0, sizeof(*file));
}

static Psf1Status read_whole_file(const char* path, uint8_t** data,
                                  size_t* size) {
  FILE*    stream;
  long     length;
  uint8_t* buffer;

  stream = fopen(path, "rb");
  if (!stream)
    return PSF1_ERROR_IO;
  if (fseek(stream, 0, SEEK_END) != 0 || (length = ftell(stream)) < 0 ||
      fseek(stream, 0, SEEK_SET) != 0) {
    fclose(stream);
    return PSF1_ERROR_IO;
  }
  buffer = (uint8_t*)malloc((size_t)length);
  if (!buffer) {
    fclose(stream);
    return PSF1_ERROR_MEMORY;
  }
  if ((size_t)length != 0 &&
      fread(buffer, 1, (size_t)length, stream) != (size_t)length) {
    free(buffer);
    fclose(stream);
    return PSF1_ERROR_IO;
  }
  fclose(stream);
  *data = buffer;
  *size = (size_t)length;
  return PSF1_OK;
}

static Psf1Status psf_file_read(const char* path, PsfFile* file) {
  uint8_t*       data = NULL;
  size_t         size = 0;
  uint32_t       reserved_size;
  uint32_t       compressed_size;
  uint32_t       expected_crc;
  const uint8_t* compressed;
  uLongf         output_size = PSF1_MAX_PROGRAM_SIZE;
  Psf1Status     status;

  memset(file, 0, sizeof(*file));
  status = read_whole_file(path, &data, &size);
  if (status != PSF1_OK)
    return status;
  if (size < PSF_HEADER_SIZE || memcmp(data, "PSF\x01", 4) != 0) {
    status = PSF1_ERROR_FORMAT;
    goto done;
  }
  reserved_size = read_le32(data + 4);
  compressed_size = read_le32(data + 8);
  expected_crc = read_le32(data + 12);
  if ((size_t)reserved_size > size - PSF_HEADER_SIZE ||
      (size_t)compressed_size > size - PSF_HEADER_SIZE - reserved_size) {
    status = PSF1_ERROR_FORMAT;
    goto done;
  }
  compressed = data + PSF_HEADER_SIZE + reserved_size;
  if ((uint32_t)crc32(0, compressed, compressed_size) != expected_crc) {
    status = PSF1_ERROR_CRC;
    goto done;
  }
  file->program = (uint8_t*)malloc(PSF1_MAX_PROGRAM_SIZE);
  if (!file->program) {
    status = PSF1_ERROR_MEMORY;
    goto done;
  }
  if (uncompress(file->program, &output_size, compressed, compressed_size) !=
      Z_OK) {
    status = PSF1_ERROR_COMPRESSION;
    goto done;
  }
  file->program_size = (size_t)output_size;

  {
    size_t tag_offset = PSF_HEADER_SIZE + reserved_size + compressed_size;
    if (size >= tag_offset + 5 && memcmp(data + tag_offset, "[TAG]", 5) == 0) {
      size_t tag_size = size - tag_offset - 5;
      file->tags = (char*)malloc(tag_size + 1);
      if (!file->tags) {
        status = PSF1_ERROR_MEMORY;
        goto done;
      }
      memcpy(file->tags, data + tag_offset + 5, tag_size);
      file->tags[tag_size] = '\0';
    }
  }
  status = PSF1_OK;

done:
  free(data);
  if (status != PSF1_OK)
    psf_file_free(file);
  return status;
}

static int tag_name_equal(const char* a, size_t a_size, const char* b) {
  size_t i;
  size_t b_size = strlen(b);
  if (a_size != b_size)
    return 0;
  for (i = 0; i < a_size; i++)
    if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
      return 0;
  return 1;
}

static int tag_value(const char* tags, const char* name, char* value,
                     size_t value_size) {
  const char* line = tags;

  if (!tags || !value || value_size == 0)
    return 0;
  while (*line) {
    const char* end = strpbrk(line, "\r\n");
    const char* equals;
    const char* name_begin = line;
    const char* name_end;
    const char* value_begin;
    const char* value_end;
    size_t      length;

    if (!end)
      end = line + strlen(line);
    equals = memchr(line, '=', (size_t)(end - line));
    if (equals) {
      while (name_begin < equals && isspace((unsigned char)*name_begin))
        name_begin++;
      name_end = equals;
      while (name_end > name_begin && isspace((unsigned char)name_end[-1]))
        name_end--;
      value_begin = equals + 1;
      while (value_begin < end && isspace((unsigned char)*value_begin))
        value_begin++;
      value_end = end;
      while (value_end > value_begin && isspace((unsigned char)value_end[-1]))
        value_end--;
      if (tag_name_equal(name_begin, (size_t)(name_end - name_begin), name)) {
        length = (size_t)(value_end - value_begin);
        if (length >= value_size)
          return -1;
        memcpy(value, value_begin, length);
        value[length] = '\0';
        return 1;
      }
    }
    line = end;
    while (*line == '\r' || *line == '\n')
      line++;
  }
  return 0;
}

static Psf1Status resolve_library_path(const char* parent, const char* library,
                                       char* path, size_t path_size) {
  const char* slash = strrchr(parent, '/');
  const char* backslash = strrchr(parent, '\\');
  const char* separator = slash;
  size_t      prefix;

  if (backslash && (!separator || backslash > separator))
    separator = backslash;
  prefix = separator ? (size_t)(separator - parent + 1) : 0;
  if (prefix + strlen(library) + 1 > path_size)
    return PSF1_ERROR_RANGE;
  memcpy(path, parent, prefix);
  strcpy(path + prefix, library);
  {
    char* cursor = path + prefix;
    while (*cursor) {
      if (*cursor == '\\')
        *cursor = '/';
      cursor++;
    }
  }
  return PSF1_OK;
}

static Psf1Status overlay_exe(const PsfFile* file, Psf1Image* image,
                              int* entry_set) {
  uint32_t pc;
  uint32_t text_address;
  uint32_t text_size;
  uint32_t sp;
  uint32_t physical;

  if (file->program_size < PSX_EXE_HEADER_SIZE ||
      memcmp(file->program, "PS-X EXE", 8) != 0)
    return PSF1_ERROR_FORMAT;
  pc = read_le32(file->program + 0x10);
  text_address = read_le32(file->program + 0x18);
  text_size = read_le32(file->program + 0x1c);
  sp = read_le32(file->program + 0x30);
  if ((size_t)text_size > file->program_size - PSX_EXE_HEADER_SIZE)
    return PSF1_ERROR_FORMAT;
  physical = text_address & 0x1fffffffu;
  if (physical >= PSF1_RAM_SIZE || text_size > PSF1_RAM_SIZE - physical)
    return PSF1_ERROR_RANGE;
  memcpy(image->ram + physical, file->program + PSX_EXE_HEADER_SIZE, text_size);
  if (physical < image->loaded_min)
    image->loaded_min = physical;
  if (physical + text_size > image->loaded_max)
    image->loaded_max = physical + text_size;
  if (!*entry_set) {
    image->initial_pc = pc;
    image->initial_sp = sp;
    *entry_set = 1;
  }
  return PSF1_OK;
}

static int tag_refresh_rate(const PsfFile* file) {
  char refresh[16];
  if (tag_value(file->tags, "_refresh", refresh, sizeof(refresh)) > 0) {
    int value = atoi(refresh);
    if (value == 50 || value == 60)
      return value;
  }
  return 0;
}

static Psf1Status load_chain(const char* path, Psf1Image* image, int* entry_set,
                             int* refresh_set, int depth) {
  PsfFile    file;
  Psf1Status status;
  char       library[512];
  char       library_path[1024];
  int        index;

  if (depth > PSF1_MAX_DEPTH)
    return PSF1_ERROR_RECURSION;
  status = psf_file_read(path, &file);
  if (status != PSF1_OK)
    return status;

  if (!*refresh_set) {
    int refresh = tag_refresh_rate(&file);
    if (refresh != 0) {
      image->refresh_rate = refresh;
      *refresh_set = 1;
    }
  }

  index = tag_value(file.tags, "_lib", library, sizeof(library));
  if (index < 0) {
    status = PSF1_ERROR_RANGE;
    goto done;
  }
  if (index > 0) {
    status =
        resolve_library_path(path, library, library_path, sizeof(library_path));
    if (status == PSF1_OK)
      status =
          load_chain(library_path, image, entry_set, refresh_set, depth + 1);
    if (status != PSF1_OK)
      goto done;
  }
  status = overlay_exe(&file, image, entry_set);
  if (status != PSF1_OK)
    goto done;

  for (index = 2;; index++) {
    char name[32];
    int  found;
    snprintf(name, sizeof(name), "_lib%d", index);
    found = tag_value(file.tags, name, library, sizeof(library));
    if (found == 0)
      break;
    if (found < 0) {
      status = PSF1_ERROR_RANGE;
      goto done;
    }
    status =
        resolve_library_path(path, library, library_path, sizeof(library_path));
    if (status == PSF1_OK)
      status =
          load_chain(library_path, image, entry_set, refresh_set, depth + 1);
    if (status != PSF1_OK)
      goto done;
  }

done:
  psf_file_free(&file);
  return status;
}

static int exe_region_refresh_rate(const PsfFile* file) {
  if (file->program_size > 0x4c + 56 &&
      memcmp(file->program + 0x4c,
             "Sony Computer Entertainment Inc. for Europe", 43) == 0)
    return 50;
  return 60;
}

Psf1Status psf1_load_file(const char* path, Psf1Image* image) {
  PsfFile    outer;
  Psf1Status status;
  int        entry_set = 0;
  int        refresh_set = 0;
  int        region_refresh;

  if (!path || !image)
    return PSF1_ERROR_ARGUMENT;
  memset(image, 0, sizeof(*image));
  image->loaded_min = PSF1_RAM_SIZE;
  status = psf_file_read(path, &outer);
  if (status != PSF1_OK)
    return status;
  region_refresh = exe_region_refresh_rate(&outer);
  psf_file_free(&outer);

  image->ram = (uint8_t*)calloc(1, PSF1_RAM_SIZE);
  if (!image->ram)
    return PSF1_ERROR_MEMORY;
  status = load_chain(path, image, &entry_set, &refresh_set, 1);
  if (status != PSF1_OK) {
    psf1_image_free(image);
    return status;
  }
  if (!refresh_set)
    image->refresh_rate = region_refresh;
  return PSF1_OK;
}

Psf1Status psf1_write_file(const char* path, const uint8_t* exe,
                           size_t exe_size, const char* tags) {
  uLongf   compressed_size;
  uint8_t* compressed;
  uint8_t  header[PSF_HEADER_SIZE];
  FILE*    stream;

  if (!path || !exe || exe_size < PSX_EXE_HEADER_SIZE ||
      exe_size > PSF1_MAX_PROGRAM_SIZE || memcmp(exe, "PS-X EXE", 8) != 0)
    return PSF1_ERROR_ARGUMENT;
  compressed_size = compressBound((uLong)exe_size);
  compressed = (uint8_t*)malloc((size_t)compressed_size);
  if (!compressed)
    return PSF1_ERROR_MEMORY;
  if (compress2(compressed, &compressed_size, exe, (uLong)exe_size,
                Z_BEST_COMPRESSION) != Z_OK) {
    free(compressed);
    return PSF1_ERROR_COMPRESSION;
  }
  memcpy(header, "PSF\x01", 4);
  write_le32(header + 4, 0);
  write_le32(header + 8, (uint32_t)compressed_size);
  write_le32(header + 12,
             (uint32_t)crc32(0, compressed, (uInt)compressed_size));
  stream = fopen(path, "wb");
  if (!stream) {
    free(compressed);
    return PSF1_ERROR_IO;
  }
  if (fwrite(header, 1, sizeof(header), stream) != sizeof(header) ||
      fwrite(compressed, 1, (size_t)compressed_size, stream) !=
          (size_t)compressed_size ||
      (tags && *tags &&
       (fwrite("[TAG]", 1, 5, stream) != 5 ||
        fwrite(tags, 1, strlen(tags), stream) != strlen(tags)))) {
    fclose(stream);
    free(compressed);
    return PSF1_ERROR_IO;
  }
  fclose(stream);
  free(compressed);
  return PSF1_OK;
}

void psf1_image_free(Psf1Image* image) {
  if (!image)
    return;
  free(image->ram);
  memset(image, 0, sizeof(*image));
}

const char* psf1_status_string(Psf1Status status) {
  switch (status) {
    case PSF1_OK:
      return "ok";
    case PSF1_ERROR_ARGUMENT:
      return "invalid argument";
    case PSF1_ERROR_IO:
      return "I/O error";
    case PSF1_ERROR_FORMAT:
      return "invalid PSF1 or PS-X EXE";
    case PSF1_ERROR_CRC:
      return "compressed program CRC mismatch";
    case PSF1_ERROR_COMPRESSION:
      return "zlib compression error";
    case PSF1_ERROR_RANGE:
      return "PSF1 image is outside PlayStation RAM";
    case PSF1_ERROR_RECURSION:
      return "PSF library recursion limit exceeded";
    case PSF1_ERROR_MEMORY:
      return "out of memory";
    default:
      return "unknown PSF1 error";
  }
}
