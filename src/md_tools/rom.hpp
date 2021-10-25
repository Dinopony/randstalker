#pragma once

#include <string>
#include <vector>
#include <map>

namespace md {

    class Code;

    class ROM
    {
    public:
        ROM(const std::string& inputPath);
        ROM(const ROM& otherROM);
        ~ROM();

        bool isValid() const { return _wasOpen; }

        uint8_t getByte(uint32_t address) const { return _byteArray[address]; }
        uint16_t getWord(uint32_t address) const { return (this->getByte(address) << 8) + this->getByte(address+1); }
        uint32_t getLong(uint32_t address) const { return (static_cast<uint32_t>(this->getWord(address)) << 16) + static_cast<uint32_t>(this->getWord(address+2)); }

        void setByte(uint32_t address, uint8_t byte);
        void setWord(uint32_t address, uint16_t word);
        void setLong(uint32_t address, uint32_t longWord);
        void setBytes(uint32_t address, std::vector<uint8_t> bytes);
        void setCode(uint32_t address, const Code& code);

        uint32_t injectBytes(const std::vector<uint8_t>& bytes, const std::string& label = "");
        uint32_t injectCode(const Code& code, const std::string& label = "");
        uint32_t reserveDataBlock(uint32_t byteCount, const std::string& label = "");

        void storeAddress(const std::string& name, uint32_t address) { _storedAddresses[name] = address; }
        uint32_t getStoredAddress(const std::string& name) { return _storedAddresses.at(name); }

        void getDataChunk(uint32_t begin, uint32_t end, std::vector<uint8_t>& output);
        void getDataChunk(uint32_t begin, uint32_t end, std::vector<uint16_t>& output);
        void getDataChunk(uint32_t begin, uint32_t end, std::vector<uint32_t>& output);

        void markChunkAsEmpty(uint32_t begin, uint32_t end);
        uint32_t countEmptyBytes() const;

        void saveAs(const std::string& outputPath);

    private:
        void updateChecksum();

        bool _wasOpen;
        char* _byteArray;
        std::map<std::string, uint32_t> _storedAddresses;

        std::vector<std::pair<uint32_t, uint32_t>> _emptyChunks;
    };

}