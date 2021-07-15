#pragma once
#include <lainlib/mutex/ticketlock.h>
#include <stdint.h>
#include <stdbool.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * @brief Abstract base class for filesystem implementations.
 * ! Do not instantiate!
 */

class FileSystem {
public:
    virtual void Init(size_t Sector, size_t Size) = 0;
    virtual const char* GetName() = 0;

    /*********** File Manipulation***********/
    virtual size_t GetFileSize(const char* FilePath) = 0;
    virtual bool FileExists(const char* FilePath) = 0;

    virtual uint8_t* Read(const char* FilePath) = 0;
    virtual size_t ReadIntoBuffer(const char* FilePath, size_t Offset, size_t Length, uint8_t* Target) = 0;

    virtual size_t Write(const char* FilePath, uint8_t* Data) = 0;
    virtual size_t WriteFromBuffer(const char* FilePath, size_t Offset, size_t Length, uint8_t* Buffer) = 0;

private:
    ticketlock_t Lock;
};