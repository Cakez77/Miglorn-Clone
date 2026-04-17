#pragma once
#include "lib.h"

#include <winbase.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// #############################################################################
//                Important Watch Patch(change to what you need)
// #############################################################################
const char* WATCH_PATH = "assets/dev/sprites";
const char* WATCH_PATH_REG = "assets/dev/sprites/*";
const char* ATLAS_OUTPUT = "assets/textures/atlas.png";


// #############################################################################
//                           Helper Functions
// #############################################################################
#include <ctype.h>
void strtoupper(char *str) 
{
  for (int i = 0; str[i]; i++) 
  { 
    str[i] = toupper((unsigned char)str[i]); 
  } 
}

char* get_next_file(const char* dir)
{
  ASSERT_MSG(dir, "No directory supplied!");
  static char fileName[MAX_PATH];

  if(!dir)
  {
    return NULL;
  }

  WIN32_FIND_DATAA fileData = {0};
  static HANDLE fileHandle = NULL;
  if(!fileHandle)
  {
    fileHandle = FindFirstFileA(dir, &fileData);
    if(fileHandle!= INVALID_HANDLE_VALUE)
    {
      memset(fileName, 0, MAX_PATH);
      memcpy(fileName, fileData.cFileName, str_len(fileData.cFileName));
      return fileName;
    }
    else
    {
      return NULL;
    }
  }

  // FineNextFileA return bool
  if(FindNextFileA(fileHandle, &fileData))
  {
    memset(fileName, 0, MAX_PATH);
    memcpy(fileName, fileData.cFileName, str_len(fileData.cFileName));
    return fileName;
  }
  else
  {
    FindClose(fileHandle);
    fileHandle = NULL;
    return NULL;
  }
}

static char* spritesFileBufferEnum = nullptr;
static char* spritesFileBufferArray = nullptr;
void write_line(char* buffer, const char* line, int& fileOffset)
{
  const int len = str_len(line);
  memcpy(buffer + fileOffset, line, len);
  fileOffset += len;
}

// #############################################################################
//                           Pack (rects) Texture
// #############################################################################
void pack_texture()
{
  // Buffer for metaprogramming
  spritesFileBufferEnum = (char*)malloc(MB(2));
  spritesFileBufferArray = (char*)malloc(MB(2));
  BumpAllocator transientStorage = make_bump_allocator(MB(5));

  // Watch directory for changes
  {
    HANDLE file = CreateFile(WATCH_PATH,
      FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      NULL);
    assert(file != INVALID_HANDLE_VALUE);

    // Event stuff to listen on
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

    uint8_t change_buf[1024];
    BOOL success = ReadDirectoryChangesW(
      file, change_buf, 1024, TRUE,
      FILE_NOTIFY_CHANGE_FILE_NAME  |
      FILE_NOTIFY_CHANGE_DIR_NAME   |
      FILE_NOTIFY_CHANGE_LAST_WRITE,
      NULL, &overlapped, NULL);

    while (true) 
    {
      // This call blocks, so we don't need to wait at all
      // it does it for us. So whenever a file changes, this 
      // will fire
      DWORD result = WaitForSingleObject(overlapped.hEvent, 0);
      if (result == WAIT_OBJECT_0) 
      {
        DWORD bytes_transferred;
        GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

        FILE_NOTIFY_INFORMATION *event = (FILE_NOTIFY_INFORMATION*)change_buf;

        // Consume Action, skip files that are not .PNG
        for (;;) 
        {
          DWORD name_len = event->FileNameLength / sizeof(wchar_t);

          // Only changed PNG files trigger the texture packer
          // because the export script in aseprite writes two files
          // this would result in two full rebuilds of the texture
          // atlas
          if(!wcsstr(event->FileName, L".png"))
          {
            goto next_wait_event;
          }

          switch (event->Action) 
          {
            case FILE_ACTION_ADDED: 
            {
              wprintf(L"       Added: %.*s\n", name_len, event->FileName);
            } break;

            case FILE_ACTION_REMOVED: 
            {
              wprintf(L"     Removed: %.*s\n", name_len, event->FileName);
            } break;

            case FILE_ACTION_MODIFIED: 
            {
              wprintf(L"    Modified: %.*s\n", name_len, event->FileName);
            } break;

            case FILE_ACTION_RENAMED_OLD_NAME: 
            {
              wprintf(L"Renamed from: %.*s\n", name_len, event->FileName);
            } break;

            case FILE_ACTION_RENAMED_NEW_NAME: 
            {
              wprintf(L"          to: %.*s\n", name_len, event->FileName);
            } break;

            default: 
            {
              printf("Unknown action!\n");
            } break;
          }

          // Are there more events to handle?
          if (event->NextEntryOffset)
          {
            *((uint8_t**)&event) += event->NextEntryOffset;
          }
          else
          {
            break;
          }
        }

        // Pack Atlas & write Sprites file
        {
          int fileOffsetEnum = 0;
          int fileOffsetArray = 0;
          memset(spritesFileBufferEnum, 0, MB(2));
          memset(spritesFileBufferArray, 0, MB(2));

          const int width = 2048;
          const int nodeCount = width;
          int rectCount = 0;
          static stbrp_node nodes[width];
          static stbrp_rect rects[width];

          while(char* fileName = get_next_file(WATCH_PATH_REG))
          {
            if(!strstr(fileName, ".png"))
            {
              continue;
            }

            int fileSize = 0;
            const char* filePath = format_text("%s/%s", WATCH_PATH, fileName);
            const unsigned char* fileData = (unsigned char*)read_file(filePath, &fileSize, &transientStorage);
            if(fileData)
            {
              int imgWidth = 0, imgHeight = 0, nChannels = 0;
              stbi_info_from_memory(fileData, fileSize, &imgWidth, &imgHeight, &nChannels);

              rects[rectCount++] = {
                .w = imgWidth,
                .h = imgHeight,
              };
            }
          }


          stbrp_context context = {};
          stbrp_init_target(&context, 2048, 2048, nodes, nodeCount);
          stbrp_pack_rects(&context, rects, rectCount);

          // Sprites file
          if(file_exists("src/sprites.h"))
          {
            delete_file("src/sprites.h");
          }

          const char* spriteDatatDefinition = 
          "\nstruct Sprite2\n"
          "{\n"
          "\tIVec2 atlasOffset;\n"
          "\tIVec2 size;\n"
          "\tIVec2 pivotOffset;\n"
          "\tint frameCount;\n"
          "};\n\n";

          // Header
          write_line(spritesFileBufferEnum, "#pragma once\n#include \"lib.h\"\n\n", fileOffsetEnum);
          write_line(spritesFileBufferEnum, spriteDatatDefinition, fileOffsetEnum);
          write_line(spritesFileBufferEnum, "enum SpriteID2 \n{\n", fileOffsetEnum);
          write_line(spritesFileBufferArray, "Sprite2 SPRITES2[SPRITE_COUNT2] =\n{\n", fileOffsetArray);

          int spriteCount;
          struct SpriteData
          {
            Vec2 atlasOffset;
            Vec2 size;
            Vec2 pivotOffset;
            int frameCount;
          };
          static SpriteData SPRITES[2000];

          // Pack Atlas
          int idx = 0;
          static unsigned char pixels[width * width * 4];
          while(char* fileNameExt = get_next_file(WATCH_PATH_REG))
          {
            if(!strstr(fileNameExt, ".png"))
            {
              continue;
            }
            
            char fileName[MAX_PATH] = {};
            const int nameLen = str_len(fileNameExt);
            memcpy(fileName, fileNameExt, nameLen - 4); // Remove extension ".png"

            // Pack file into atlas
            int imgWidth = 0, imgHeight = 0, nChannels = 0, iniFileSize;
            const char* filePath = format_text("%s/%s.png", WATCH_PATH, fileName);
            const char* iniPath = format_text("%s/%s.txt", WATCH_PATH, fileName);
            char* spritePixels = (char*)stbi_load(filePath, &imgWidth, &imgHeight, &nChannels, 4);
            ConfigINI iniData = parse_ini(iniPath, &transientStorage);
            if(spritePixels)
            {
              const stbrp_rect& rect = rects[idx++];

              // Uppercase all characters
              strtoupper(fileName);
              char* enumName = format_text("SPRITE_%s", fileName);

              int frameCount = 1;
              int pivotOffsetX = 0;
              int pivotOffsetY = 0;
              int spriteSizeX = 0;
              int spriteSizeY = 0;

              // Read data from ini file
              int tagCount = 0;
              for(INIField& field : iniData.fields)
              {
                // Needs to be the last field in the ini
                if(strstr(field.name, "tagName"))
                {
                  // Empty Tags for sprites with no Tags
                  if(str_len(field.value) > 0)
                  {
                    strtoupper(field.value);
                    enumName = format_text("SPRITE_%s_%s", fileName, field.value);
                  }

                  // Enum Definition
                  char* enumLine = format_text("\t%s,\n", enumName);
                  write_line(spritesFileBufferEnum, enumLine, fileOffsetEnum);

                  // Sprite Data in array
                  const int atlasOffsetX = rect.x;
                  const int atlasOffsetY = rect.y;
                  char* spriteData = 
                    format_text("\t[%s] = {.atlasOffset={%d, %d}, .size={%d, %d}, .pivotOffset={%d, %d}, .frameCount=%d},\n",
                      enumName, 
                      atlasOffsetX, atlasOffsetY, 
                      spriteSizeX, spriteSizeY, 
                      pivotOffsetX, pivotOffsetY, 
                      frameCount);
                  write_line(spritesFileBufferArray, spriteData, fileOffsetArray);
                }
                else if(strstr(field.name, "frameCount"))
                {
                  frameCount = atoi(field.value);
                }
                else if(strstr(field.name, "pivotOffsetX"))
                {
                  pivotOffsetX = atoi(field.value);
                }
                else if(strstr(field.name, "pivotOffsetY"))
                {
                  pivotOffsetY = atoi(field.value);
                }
                else if(strstr(field.name, "spriteSizeX"))
                {
                  spriteSizeX = atoi(field.value);
                }
                else if(strstr(field.name, "spriteSizeY"))
                {
                  spriteSizeY = atoi(field.value);
                }
              }

              // Copy the pixels into Atlas
              const int col = rect.x * 4;
              for(int i = 0; i < imgHeight; i++)
              {
                const int atlasRow = (rect.y + i) * width * 4;
                const int spriteRow = i * imgWidth * 4;
                memcpy(pixels + atlasRow + col, spritePixels + spriteRow, sizeof(char) * 4 * imgWidth);
              }

              transientStorage.used = 0;
            }
          }

          stbi_write_png("assets/textures/textureAtlas.png", width, width, 4, pixels, width * 4);

          // Write the end of the enum & sprites array
          write_line(spritesFileBufferEnum, "\tSPRITE_COUNT2\n};\n\n", fileOffsetEnum);
          write_line(spritesFileBufferArray, "\n};\n\n", fileOffsetArray);

          SDL_IOStream* spritesFile = SDL_IOFromFile("src/sprites.h", "w");
          SDL_WriteIO(spritesFile, spritesFileBufferEnum, fileOffsetEnum);
          SDL_WriteIO(spritesFile, spritesFileBufferArray, fileOffsetArray);

          SDL_CloseIO(spritesFile);
        }
        
        // Queue the next event
        next_wait_event:
        BOOL success = ReadDirectoryChangesW(
              file, change_buf, 1024, TRUE,
              FILE_NOTIFY_CHANGE_FILE_NAME  |
              FILE_NOTIFY_CHANGE_DIR_NAME   |
              FILE_NOTIFY_CHANGE_LAST_WRITE,
              NULL, &overlapped, NULL);
      }
    }
  }

  free(spritesFileBufferEnum);
  free(spritesFileBufferArray);
}