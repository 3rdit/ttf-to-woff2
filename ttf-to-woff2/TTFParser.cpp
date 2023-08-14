#include "TTFParser.hpp"
#include <fstream>
#include <iostream>

namespace TTFParser {
    bool TTFParser::parseHeadTable(uint32_t offset) {
        // Ensure that there is enough data for the 'head' table (total 54 bytes worth of data)
        if (fontData.size() < offset + 54) {
            std::cerr << "Error: not enough data for 'head' headTable." << std::endl;
            return false;
        }

        // Parse version and check if it's supported in this parser
        headTable.version = fixedToFloat(swapEndian32(*(int32_t*)&fontData[offset]));
        if (headTable.version != 1.0f) {
            std::cerr << "Error: Unsupported 'head' headTable version." << std::endl;
            return false;
        }

        headTable.fontRevision = fixedToFloat(swapEndian32(*(int32_t*)&fontData[offset + 4]));
        headTable.checkSumAdjustment = swapEndian32(*(uint32_t*)&fontData[offset + 8]);
        headTable.magicNumber = swapEndian32(*(uint32_t*)&fontData[offset + 12]);

        if (headTable.magicNumber != 0x5F0F3CF5) { // https://learn.microsoft.com/en-us/typography/opentype/spec/head
            std::cerr << "Error: Invalid magic number in 'head' headTable." << std::endl;
            return false;
        }

        headTable.flags = swapEndian16(*(uint16_t*)&fontData[offset + 16]);
        headTable.unitsPerEm = swapEndian16(*(uint16_t*)&fontData[offset + 18]);
        headTable.created = swapEndian64(*(int64_t*)&fontData[offset + 20]);
        headTable.modified = swapEndian64(*(int64_t*)&fontData[offset + 28]);
        headTable.xMin = swapEndian16(*(int16_t*)&fontData[offset + 36]);
        headTable.yMin = swapEndian16(*(int16_t*)&fontData[offset + 38]);
        headTable.xMax = swapEndian16(*(int16_t*)&fontData[offset + 40]);
        headTable.yMax = swapEndian16(*(int16_t*)&fontData[offset + 42]);
        headTable.macStyle = swapEndian16(*(uint16_t*)&fontData[offset + 44]);
        headTable.lowestRecPPEM = swapEndian16(*(uint16_t*)&fontData[offset + 46]);
        headTable.fontDirectionHint = swapEndian16(*(int16_t*)&fontData[offset + 48]);
        headTable.indexToLocFormat = swapEndian16(*(int16_t*)&fontData[offset + 50]);
        headTable.glyphDataFormat = swapEndian16(*(int16_t*)&fontData[offset + 52]);

        return true;
    }

    bool TTFParser::parseNameTable(uint32_t offset, NameTable& table) {
        if (fontData.size() < offset + 6) { // 6 bytes for the header
            std::cerr << "Error: Not enough data for 'name' table header." << std::endl;
            return false;
        }

        table.format = swapEndian16(*(uint16_t*)&fontData[offset]);
        table.count = swapEndian16(*(uint16_t*)&fontData[offset + 2]);
        table.stringOffset = swapEndian16(*(uint16_t*)&fontData[offset + 4]);

        offset += 6; // Move past the header

        for (uint16_t i = 0; i < table.count; ++i) {
            if (fontData.size() < offset + 12) { // 12 bytes for each name record
                std::cerr << "Error: Not enough data for name record " << i << "." << std::endl;
                return false;
            }

            NameRecord record;
            record.platformID = swapEndian16(*(uint16_t*)&fontData[offset]);
            record.encodingID = swapEndian16(*(uint16_t*)&fontData[offset + 2]);
            record.languageID = swapEndian16(*(uint16_t*)&fontData[offset + 4]);
            record.nameID = swapEndian16(*(uint16_t*)&fontData[offset + 6]);
            record.length = swapEndian16(*(uint16_t*)&fontData[offset + 8]);
            record.offset = swapEndian16(*(uint16_t*)&fontData[offset + 10]);

            // Navigate to the string in the string storage and read it
            uint32_t stringStart = offset + table.stringOffset + record.offset;
            if (fontData.size() < stringStart + record.length) {
                std::cerr << "Error: Not enough data for name string of record " << i << "." << std::endl;
                return false;
            }

            if (record.encodingID == 0 || record.encodingID == 1) { // ASCII or UTF-16
                for (uint16_t j = 0; j < record.length; ++j) {
                    record.nameString += fontData[stringStart + j];
                }
            }
            else if (record.encodingID == 2) { // UTF-16
                for (uint16_t j = 0; j < record.length; j += 2) {
                    char16_t ch = swapEndian16(*(char16_t*)&fontData[stringStart + j]);
                    record.nameString += std::string((char*)&ch, 2);
                }
            }
            else {
                std::cerr << "Warning: Unsupported encoding in name record " << i << "." << std::endl;
            }

            table.nameRecords.push_back(record);
            offset += 12; // Move to the next name record
        }

        return true;
    }

    bool TTFParser::parseHheaTable(uint32_t offset, HheaTable& table) {
        if (fontData.size() < offset + 36) { // The 'hhea' table should be at least 36 bytes
            std::cerr << "Error: not enough data for 'hhea' table." << std::endl;
            return false;
        }

        table.version = fixedToFloat(swapEndian32(*(int32_t*)&fontData[offset]));
        offset += 4;

        table.ascent = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.descent = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.lineGap = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.advanceWidthMax = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        table.minLeftSideBearing = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.minRightSideBearing = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.xMaxExtent = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.caretSlopeRise = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.caretSlopeRun = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        table.caretOffset = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        for (int i = 0; i < 4; ++i) {
            table.reserved[i] = swapEndian16(*(int16_t*)&fontData[offset]);
            offset += 2;
        }

        table.metricDataFormat = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        if (table.metricDataFormat != 0) {
            std::cerr << "Error: Unsupported metric data format in 'hhea' table." << std::endl;
            return false;
        }

        table.numOfLongHorMetrics = swapEndian16(*(uint16_t*)&fontData[offset]);

        return true;
    }

    bool TTFParser::parseHmtxTable(uint32_t offset, uint16_t numOfLongHorMetrics, std::vector<GlyphMetrics>& metrics) {
        if (fontData.size() < offset + numOfLongHorMetrics * 4 + (numGlyphs - numOfLongHorMetrics) * 2) {
            std::cerr << "Error: not enough data for 'hmtx' table." << std::endl;
            return false;
        }

        metrics.resize(numGlyphs);

        std::cout << "numGlyphs: " << numGlyphs << std::endl;
        std::cout << "numOfLongHorMetrics: " << numOfLongHorMetrics << std::endl;

        for (uint16_t i = 0; i < numOfLongHorMetrics; ++i) {
            if (offset + 4 > fontData.size()) {
                std::cerr << "Error: Exceeded font data while reading 'hmtx' metrics." << std::endl;
                return false;
            }

            std::cout << "Accessing metrics[" << i << "]" << std::endl;

            metrics[i].advanceWidth = swapEndian16(*(uint16_t*)&fontData[offset]);
            metrics[i].lsb = swapEndian16(*(uint16_t*)&fontData[offset + 2]);

            offset += 4;

            // Optional Diagnostic print - might produce a lot of output for fonts with many glyphs.
             std::cout << "Metric " << i << ": Advance Width = " << metrics[i].advanceWidth
                 << ", Left Side Bearing = " << metrics[i].lsb << std::endl;
        }

        // If there are more glyphs than numOfLongHorMetrics, the remaining glyphs will use the last advanceWidth from the above loop
        uint16_t lastAdvanceWidth = (numOfLongHorMetrics > 0) ? metrics[numOfLongHorMetrics - 1].advanceWidth : 0;

        for (uint16_t i = numOfLongHorMetrics; i < numGlyphs; ++i) {
            metrics[i].advanceWidth = lastAdvanceWidth;
            metrics[i].lsb = swapEndian16(*(int16_t*)&fontData[offset]);
            offset += 2;
        }

        return true;
    }

    bool TTFParser::parseCmapFormat0(uint32_t offset, CmapFormat0& table) {
        // Check boundaries
        if (offset + 256 > fontData.size()) {
            std::cerr << "Error: cmap format 0 table is shorter than expected." << std::endl;
            return false;
        }

        // Read the glyphIdArray
        for (int i = 0; i < 256; ++i) {
            table.glyphIdArray[i] = fontData[offset + i];

            // Optional: Check if the glyph index is in range
            if (table.glyphIdArray[i] >= numGlyphs) {
                std::cerr << "Error: Glyph index in cmap format 0 is out of range." << std::endl;
                return false;
            }
        }

        return true;
    }

    bool TTFParser::parseCmapFormat2(uint32_t offset, CmapFormat2& table) {
        // Check initial boundaries
        if (offset + 512 > fontData.size()) { // 2 bytes for format, length, and language + 256 for subHeaderKeys
            std::cerr << "Error: cmap format 2 table is too short for initial data." << std::endl;
            return false;
        }

        table.length = swapEndian16(*(uint16_t*)&fontData[offset + 2]);
        table.language = swapEndian16(*(uint16_t*)&fontData[offset + 4]);

        offset += 6;

        for (int i = 0; i < 256; ++i) {
            table.subHeaderKeys[i] = fontData[offset + i];
        }
        offset += 256;

        // Extract SubHeaders
        uint16_t maxSubHeaderIndex = 0;
        for (const auto& key : table.subHeaderKeys) {
            uint16_t subHeaderIndex = key / 8;
            if (subHeaderIndex > maxSubHeaderIndex) {
                maxSubHeaderIndex = subHeaderIndex;
            }
        }

        for (uint16_t i = 0; i <= maxSubHeaderIndex; ++i) {
            if (offset + 8 > fontData.size()) {
                std::cerr << "Error: cmap format 2 table is too short for subHeaders." << std::endl;
                return false;
            }

            SubHeader sh;
            sh.firstCode = swapEndian16(*(uint16_t*)&fontData[offset]);
            sh.entryCount = swapEndian16(*(uint16_t*)&fontData[offset + 2]);
            sh.idDelta = swapEndian16(*(uint16_t*)&fontData[offset + 4]);
            sh.idRangeOffset = swapEndian16(*(uint16_t*)&fontData[offset + 6]);

            table.subHeaders.push_back(sh);

            offset += 8;
        }

        // Continue with glyphIndexArray parsing...

        return true;
    }

    bool TTFParser::parseCmapFormat4(uint32_t offset, CmapFormat4& table) {
        table.segCountX2 = swapEndian16(*(uint16_t*)&fontData[offset + 2]);

        if (table.segCountX2 % 2 != 0) {
            std::cerr << "Error: segCountX2 is not an even number." << std::endl;
            return false;
        }

        uint16_t segCount = table.segCountX2 / 2;

        if (fontData.size() < offset + 14 + 8 * segCount) {
            std::cerr << "Error: cmap format 4 table is shorter than expected." << std::endl;
            return false;
        }

        // Skip 6 bytes to reach endCount array
        offset += 14;
        for (uint16_t i = 0; i < segCount; ++i) {
            table.endCount.push_back(swapEndian16(*(uint16_t*)&fontData[offset]));
            offset += 2;
        }

        // Skip reservedPad
        offset += 2;

        for (uint16_t i = 0; i < segCount; ++i) {
            table.startCount.push_back(swapEndian16(*(uint16_t*)&fontData[offset]));
            offset += 2;
        }

        for (uint16_t i = 0; i < segCount; ++i) {
            table.idDelta.push_back(swapEndian16(*(uint16_t*)&fontData[offset]));
            offset += 2;
        }

        for (uint16_t i = 0; i < segCount; ++i) {
            table.idRangeOffset.push_back(swapEndian16(*(uint16_t*)&fontData[offset]));
            offset += 2;
        }

        // Calculate the glyph indices
        for (uint16_t i = 0; i < segCount; ++i) {
            for (uint16_t charCode = table.startCount[i]; charCode <= table.endCount[i]; ++charCode) {
                uint16_t glyphIndex;
                if (table.idRangeOffset[i] == 0) {
                    glyphIndex = (charCode + table.idDelta[i]) % 65536; // Use modulo to ensure the result wraps around in a 16-bit integer.
                }
                else {
                    uint16_t offset = (charCode - table.startCount[i]) * 2 + table.idRangeOffset[i];
                    uint32_t indexPosition = offset + 2 * i + 14 + 6 * segCount;
                    if (indexPosition >= fontData.size()) {
                        std::cerr << "Error: Calculated position for glyph index exceeds font data size." << std::endl;
                        return false;
                    }
                    glyphIndex = swapEndian16(*(uint16_t*)&fontData[offset]);
                }

                if (glyphIndex >= numGlyphs) { // Assuming you've already parsed the 'maxp' table and stored the number of glyphs in a member called `numGlyphs`.
                    std::cerr << "Error: Calculated glyph index is out of range." << std::endl;
                    return false;
                }

                // Store glyphIndex in the glyphIndices vector
                table.glyphIndices.push_back(glyphIndex);
            }
        }

        for (uint16_t i = 0; i < segCount; ++i) {
            if (table.startCount[i] > table.endCount[i]) {
                std::cerr << "Error: startCount is greater than endCount for segment " << i << std::endl;
                return false;
            }
        }

        return true;
    }

    bool TTFParser::parseCmapFormat6(uint32_t offset, CmapFormat6& table) {
        table.firstCode = swapEndian16(*(uint16_t*)&fontData[offset]);
        table.entryCount = swapEndian16(*(uint16_t*)&fontData[offset + 2]);

        if (fontData.size() < offset + 4 + 2 * table.entryCount) {
            std::cerr << "Error: cmap format 6 table is shorter than expected." << std::endl;
            return false;
        }

        offset += 4;
        for (uint16_t i = 0; i < table.entryCount; ++i) {
            table.glyphIdArray.push_back(swapEndian16(*(uint16_t*)&fontData[offset]));
            offset += 2;
        }

        return true;
    }

    bool TTFParser::parseCmapFormat8(uint32_t offset, CmapFormat8& table) {
        // TODO: Implement the parsing for format 8 if needed.
        return true;
    }

    bool TTFParser::parseCmapFormat10(uint32_t offset, CmapFormat10& table) {
        table.startCharCode = swapEndian32(*(uint32_t*)&fontData[offset]);
        table.numChars = swapEndian32(*(uint32_t*)&fontData[offset + 4]);

        if (fontData.size() < offset + 8 + 2 * table.numChars) {
            std::cerr << "Error: cmap format 10 table is shorter than expected." << std::endl;
            return false;
        }

        offset += 8;
        for (uint32_t i = 0; i < table.numChars; ++i) {
            table.glyphs.push_back(swapEndian16(*(uint16_t*)&fontData[offset]));
            offset += 2;
        }

        return true;
    }

    bool TTFParser::parseCmapFormat12(uint32_t offset, CmapFormat12& table) {
        table.numGroups = swapEndian32(*(uint32_t*)&fontData[offset]);

        if (fontData.size() < offset + 4 + 12 * table.numGroups) {
            std::cerr << "Error: cmap format 12 table is shorter than expected." << std::endl;
            return false;
        }

        offset += 4;
        for (uint32_t i = 0; i < table.numGroups; ++i) {
            GroupFormat12 group;
            group.startCharCode = swapEndian32(*(uint32_t*)&fontData[offset]);
            group.endCharCode = swapEndian32(*(uint32_t*)&fontData[offset + 4]);
            group.startGlyphID = swapEndian32(*(uint32_t*)&fontData[offset + 8]);
            table.groups.push_back(group);
            offset += 12;
        }

        return true;
    }

    bool TTFParser::parseCmapFormat13(uint32_t offset, CmapFormat13& table) {
        table.numGroups = swapEndian32(*(uint32_t*)&fontData[offset]);

        if (fontData.size() < offset + 4 + 12 * table.numGroups) {
            std::cerr << "Error: cmap format 13 table is shorter than expected." << std::endl;
            return false;
        }

        offset += 4;
        for (uint32_t i = 0; i < table.numGroups; ++i) {
            GroupFormat13 group;
            group.startCharCode = swapEndian32(*(uint32_t*)&fontData[offset]);
            group.endCharCode = swapEndian32(*(uint32_t*)&fontData[offset + 4]);
            group.glyphID = swapEndian32(*(uint32_t*)&fontData[offset + 8]);
            table.groups.push_back(group);
            offset += 12;
        }

        return true;
    }

    bool TTFParser::parseCmapFormat14(uint32_t offset, CmapFormat14& table) {
        table.format = swapEndian16(*(uint16_t*)&fontData[offset]);
        table.length = swapEndian32(*(uint32_t*)&fontData[offset + 2]);
        table.numVarSelectorRecords = swapEndian32(*(uint32_t*)&fontData[offset + 6]);

        uint32_t currentOffset = offset + 10; // Start of the varSelector array

        for (uint32_t i = 0; i < table.numVarSelectorRecords; ++i) {
            VarSelectorRecord record;
            record.varSelector = (fontData[currentOffset] << 16) | (fontData[currentOffset + 1] << 8) | fontData[currentOffset + 2];
            record.defaultUVSOffset = swapEndian32(*(uint32_t*)&fontData[currentOffset + 3]);
            record.nonDefaultUVSOffset = swapEndian32(*(uint32_t*)&fontData[currentOffset + 7]);
            table.varSelectors.push_back(record);
            currentOffset += 11; // Move to the next VarSelectorRecord
        }

        for (auto& selectorRecord : table.varSelectors) {
            // Parse default UVS table if present
            if (selectorRecord.defaultUVSOffset) {
                uint32_t uvsOffset = offset + selectorRecord.defaultUVSOffset;
                if (uvsOffset + 4 > fontData.size()) {
                    std::cerr << "Error: Default UVS table offset exceeds font data size." << std::endl;
                    return false;
                }

                uint32_t numUnicodeValueRanges = swapEndian32(*(uint32_t*)&fontData[uvsOffset]);
                uvsOffset += 4;

                for (uint32_t j = 0; j < numUnicodeValueRanges; ++j) {
                    if (uvsOffset + 4 > fontData.size()) {
                        std::cerr << "Error: Unicode value range exceeds font data size." << std::endl;
                        return false;
                    }

                    UnicodeValueRange range;
                    range.startUnicodeValue = (fontData[uvsOffset] << 16) | (fontData[uvsOffset + 1] << 8) | fontData[uvsOffset + 2];
                    range.additionalCount = fontData[uvsOffset + 3];
                    selectorRecord.defaultUVSTable.push_back(range);
                    uvsOffset += 4; // Move to next UnicodeValueRange
                }
            }

            // Parse non-default UVS table if present
            if (selectorRecord.nonDefaultUVSOffset) {
                uint32_t uvsOffset = offset + selectorRecord.nonDefaultUVSOffset;
                if (uvsOffset + 4 > fontData.size()) {
                    std::cerr << "Error: Non-default UVS table offset exceeds font data size." << std::endl;
                    return false;
                }

                uint32_t numUVSMappings = swapEndian32(*(uint32_t*)&fontData[uvsOffset]);
                uvsOffset += 4;

                for (uint32_t j = 0; j < numUVSMappings; ++j) {
                    if (uvsOffset + 5 > fontData.size()) {
                        std::cerr << "Error: UVS mapping exceeds font data size." << std::endl;
                        return false;
                    }

                    UVSMapping mapping;
                    mapping.unicodeValue = (fontData[uvsOffset] << 16) | (fontData[uvsOffset + 1] << 8) | fontData[uvsOffset + 2];
                    mapping.glyphID = swapEndian16(*(uint16_t*)&fontData[uvsOffset + 3]);
                    selectorRecord.nonDefaultUVSTable.push_back(mapping);

                    uvsOffset += 5; // Move to next UVSMapping
                }
            }
        }

        return true;
    }

    bool TTFParser::loadFromFile(const std::string& filename) {
        // Load the entire TTF file into memory
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open TTF file: " << filename << std::endl;
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        fontData.resize(size);
        if (!file.read((char*)fontData.data(), size)) {
            std::cerr << "Failed to read TTF file: " << filename << std::endl;
            return false;
        }

        // Parse the offset table
        return readOffsetTable();
    }

    bool TTFParser::readOffsetTable() {
        // Check font data size
        if (fontData.size() < sizeof(OffsetTable)) {
            std::cerr << "Invalid TTF file: insufficient data for offset table" << std::endl;
            return false;
        }

        // Read the offset table data
        size_t offset = 0;
        offsetTable.sfntVersion = swapEndian32(*(uint32_t*)&fontData[offset]); offset += 4;
        offsetTable.numTables = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
        offsetTable.searchRange = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
        offsetTable.entrySelector = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
        offsetTable.rangeShift = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;

        // Read table directory entries
        for (uint16_t i = 0; i < offsetTable.numTables; ++i) {
            if (fontData.size() < offset + sizeof(TableDirectoryEntry)) {
                std::cerr << "Invalid TTF file: insufficient data for table directory entry" << std::endl;
                return false;
            }

            TableDirectoryEntry entry;
            entry.tag = *(uint32_t*)&fontData[offset]; offset += 4;  // Do not byte-swap the tag
            entry.checkSum = swapEndian32(*(uint32_t*)&fontData[offset]); offset += 4;
            entry.offset = swapEndian32(*(uint32_t*)&fontData[offset]); offset += 4;
            entry.length = swapEndian32(*(uint32_t*)&fontData[offset]); offset += 4;

            offsetTable.tableDirectoryEntries.push_back(entry);

            // Load the table data for this entry
            if (fontData.size() < entry.offset + entry.length) {
                std::cerr << "Invalid TTF file: insufficient data for table content" << std::endl;
                return false;
            }
            std::vector<uint8_t> table(fontData.begin() + entry.offset, fontData.begin() + entry.offset + entry.length);

            char tagStr[5];
            std::memcpy(tagStr, &entry.tag, 4);
            tagStr[4] = '\0';
            tableData[tagStr] = table;
        }

        return true;
    }

    const std::vector<uint8_t>& TTFParser::getTableData(const std::string& tag) const {
        static std::vector<uint8_t> empty;
        auto it = tableData.find(tag);
        return (it != tableData.end()) ? it->second : empty;
    }

    uint32_t TTFParser::getTableLength(const std::string& tableName) {
        uint32_t tag = *(uint32_t*)tableName.c_str();  // Convert the string to its uint32_t representation
        for (const auto& entry : offsetTable.tableDirectoryEntries) {
            if (entry.tag == tag) {
                return entry.length;
            }
        }
        return 0;  // Return 0 if table not found
    }

    bool TTFParser::parseFVarTable(uint32_t offset, FVarTable& fvar) {
        // Helper functions you've already defined: swapEndian32, swapEndian16, fixedToFloat

        std::cout << "Raw 'fvar' data (first 100 bytes):" << std::endl;
        for (int i = 0; i < 100; ++i) {
            printf("%02X ", (uint8_t)fontData[offset + i]);
            if (i % 16 == 15) printf("\n");
        }
        printf("\n");

        // Parsing the header
        uint16_t majorVersion = swapEndian16(*(uint16_t*)&fontData[offset]);
        uint16_t minorVersion = swapEndian16(*(uint16_t*)&fontData[offset + 2]);
        uint16_t axesArrayOffset = swapEndian16(*(uint16_t*)&fontData[offset + 4]);
        uint16_t reserved = swapEndian16(*(uint16_t*)&fontData[offset + 6]);
        uint16_t axisCount = swapEndian16(*(uint16_t*)&fontData[offset + 8]);
        uint16_t axisSize = swapEndian16(*(uint16_t*)&fontData[offset + 10]);
        uint16_t instanceCount = swapEndian16(*(uint16_t*)&fontData[offset + 12]);
        uint16_t instanceSize = swapEndian16(*(uint16_t*)&fontData[offset + 14]);


        uint32_t currentOffset = offset + axesArrayOffset;

        std::cout << "Raw axis values (before fixedToFloat):" << std::endl;
        printf("Min: %08X\n", swapEndian32(*(int32_t*)&fontData[currentOffset + 4]));
        printf("Default: %08X\n", swapEndian32(*(int32_t*)&fontData[currentOffset + 8]));
        printf("Max: %08X\n", swapEndian32(*(int32_t*)&fontData[currentOffset + 12]));


        // Parsing the VariationAxisRecord array
        for (uint16_t i = 0; i < axisCount; ++i) {
            uint32_t axisTag = swapEndian32(*(uint32_t*)&fontData[currentOffset]);
            float minValue = fixedToFloat(swapEndian32(*(int32_t*)&fontData[currentOffset + 4]));
            float defaultValue = fixedToFloat(swapEndian32(*(int32_t*)&fontData[currentOffset + 8]));
            float maxValue = fixedToFloat(swapEndian32(*(int32_t*)&fontData[currentOffset + 12]));

            // Check if values are in the expected range
            if (minValue > defaultValue || defaultValue > maxValue) {
                std::cerr << "Axis " << i << " has inconsistent values:" << std::endl;
                std::cerr << "Min Value: " << minValue << std::endl;
                std::cerr << "Default Value: " << defaultValue << std::endl;
                std::cerr << "Max Value: " << maxValue << std::endl;
                return false;
            }

            // Move by the known size of a VariationAxisRecord, not the axisSize
            currentOffset += 20;  // 4 bytes for tag, 12 bytes for min/default/max, 2 bytes for flags, 2 bytes for axisNameID
        }

        // Parsing the InstanceRecord array
        for (uint16_t i = 0; i < instanceCount; ++i) {
            // Only reading the necessary fields for demonstration
            uint16_t subfamilyNameID = swapEndian16(*(uint16_t*)&fontData[currentOffset]);
            uint16_t flags = swapEndian16(*(uint16_t*)&fontData[currentOffset + 2]);

            // Skipping to coordinates
            currentOffset += 4; // The first two fields of the InstanceRecord take up 4 bytes total
            for (uint16_t j = 0; j < axisCount; ++j) {
                float coordinate = fixedToFloat(swapEndian32(*(int32_t*)&fontData[currentOffset]));
                currentOffset += sizeof(float); // Since a coordinate is a float (Fixed type)
            }
            // If postScriptNameID is present, it can be parsed here.

            currentOffset += instanceSize - (axisCount * sizeof(float) + 4); // Adjusting for the size of the InstanceRecord
        }

        return true;
    }

    bool TTFParser::parseGlyph(uint32_t glyphOffset, SimpleGlyph& glyph) {
        static int compoundGlyphCount = 0;

        if (glyphOffset >= fontData.size()) {
            return false; // Offset out of range
        }

        uint32_t offset = glyphOffset;

        // Check for sufficient data for header
        if (offset + 10 > fontData.size()) {
            return false;
        }

        // Read the glyph header
        int16_t numberOfContours = swapEndian16(*(int16_t*)&fontData[offset]);
        offset += 2;

        // Bounding box values which we'll skip for now
        offset += 8;

        // If numberOfContours is positive, the glyph is simple
        if (numberOfContours > 0) {
            glyph.numberOfContours = numberOfContours;

            // Check for sufficient data for end points of contours
            if (offset + 2 * numberOfContours > fontData.size()) {
                return false;
            }

            // Read the end points of each contour
            for (int i = 0; i < numberOfContours; ++i) {
                uint16_t endPoint = swapEndian16(*(uint16_t*)&fontData[offset]);
                glyph.endPointOfContours.push_back(endPoint);
                offset += 2;
            }

            // Check for sufficient data for instruction length
            if (offset + 2 > fontData.size()) {
                return false;
            }

            // Read the instruction length
            glyph.instructionLength = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            // Check for sufficient data for instructions
            if (offset + glyph.instructionLength > fontData.size()) {
                return false;
            }

            // Read the instructions
            glyph.instructions.resize(glyph.instructionLength);
            std::memcpy(glyph.instructions.data(), &fontData[offset], glyph.instructionLength);
            offset += glyph.instructionLength;

            // TODO: Read the flags and points for the glyph

        }

        else if (numberOfContours == -1) {
            CompoundGlyph compoundGlyph;
            if (!parseCompoundGlyph(offset, compoundGlyph)) {
                std::cerr << "Failed to parse compound glyph at offset " << glyphOffset << "." << std::endl;
                return false;
            }

            compoundGlyphCount++;
            std::cout << "Successfully parsed compound glyph #" << compoundGlyphCount
                << " at offset " << glyphOffset << "." << std::endl;
        }

        else {
            compoundGlyphCount++;

            std::cout << "This glyph is empty (e.g., space character), glyph #" << compoundGlyphCount
                << " at offset " << glyphOffset << "." << std::endl;

            // The glyph is empty (e.g., space character)
        }

        return true;
    }

    bool TTFParser::parseCompoundGlyph(uint32_t& offset, CompoundGlyph& glyph) {
        bool continueParsing = true;
        uint16_t flags;

        while (continueParsing) {
            // Ensure there's enough data for the flags and glyph index
            if (offset + 4 > fontData.size()) {
                return false;
            }

            flags = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            uint16_t glyphIndex = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            glyph.components.push_back(CompoundComponent{ glyphIndex }); // Store the glyph index

            std::cout << "Parsed compound glyph. Flags: " << flags
                << ", Glyph Index: " << glyphIndex << std::endl;

            // Handle ARG_1_AND_2_ARE_WORDS flag
            int arg1, arg2;
            if (flags & ARG_1_AND_2_ARE_WORDS) {
                arg1 = swapEndian16(*(int16_t*)&fontData[offset]);
                offset += 2;
                arg2 = swapEndian16(*(int16_t*)&fontData[offset]);
                offset += 2;

                std::cout << "Arguments are words. Arg1: " << arg1
                    << ", Arg2: " << arg2 << std::endl;
            }
            else {
                arg1 = (int8_t)fontData[offset++];
                arg2 = (int8_t)fontData[offset++];

                std::cout << "Arguments are bytes. Arg1: " << arg1
                    << ", Arg2: " << arg2 << std::endl;
            }

            if (flags & WE_HAVE_A_SCALE) {
                if (offset + 2 > fontData.size()) {
                    return false;
                }

                float scale = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;

                std::cout << "Single scale: " << scale << std::endl;
            }
            else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                if (offset + 4 > fontData.size()) {
                    return false;
                }

                float scaleX = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;
                float scaleY = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;

                std::cout << "Scale X: " << scaleX << ", Scale Y: " << scaleY << std::endl;
            }
            else if (flags & WE_HAVE_A_TWO_BY_TWO) {
                if (offset + 8 > fontData.size()) {
                    return false;
                }

                float scaleX = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;
                float skew0 = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;
                float skew1 = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;
                float scaleY = f2Dot14ToFloat(swapEndian16(*(int16_t*)&fontData[offset]));
                offset += 2;

                std::cout << "2x2 Matrix - Scale X: " << scaleX << ", Skew 0: " << skew0
                    << ", Skew 1: " << skew1 << ", Scale Y: " << scaleY << std::endl;
            }

            // Check if there are more components to read
            continueParsing = (flags & MORE_COMPONENTS) != 0;
        }

        // Handle the WE_HAVE_INSTRUCTIONS flag after all components are parsed
        if (flags & WE_HAVE_INSTRUCTIONS) {
            if (offset + 2 > fontData.size()) {
                return false; // Insufficient data for instruction length
            }

            uint16_t instructionLength = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            if (offset + instructionLength > fontData.size()) {
                return false; // Insufficient data for instructions
            }

            // For now, we'll just skip the instructions.
            offset += instructionLength;

            std::cout << "Parsed instructions of length: " << instructionLength << std::endl;
        }

        return true;
    }
    
    bool TTFParser::parseCmapTable(uint32_t offset, CmapTable& cmap) {
        cmap.version = swapEndian16(*(uint16_t*)&fontData[offset]);
        cmap.numTables = swapEndian16(*(uint16_t*)&fontData[offset + 2]);

        uint32_t currentOffset = offset + 4; // After version and numTables

        for (int i = 0; i < cmap.numTables; ++i) {
            uint16_t platformID = swapEndian16(*(uint16_t*)&fontData[currentOffset]);
            uint16_t encodingID = swapEndian16(*(uint16_t*)&fontData[currentOffset + 2]);
            uint32_t subtableOffset = offset + swapEndian32(*(uint32_t*)&fontData[currentOffset + 4]);

            uint16_t format = swapEndian16(*(uint16_t*)&fontData[subtableOffset]);
            std::cout << "Subtable format after swapping: " << format << std::endl;

            std::cout << "Raw bytes at subtableOffset: ";
            for (int j = 0; j < 4; ++j) {
                printf("%02X ", (uint8_t)fontData[subtableOffset + j]);
            }
            printf("\n");

            switch (format) {
            case 0:
            {
                CmapFormat0 subtable;
                subtable.format = format;

                if (!parseCmapFormat0(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 0" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat0>(subtable));
                break;
            }
            case 2:
            {
                CmapFormat2 subtable;
                subtable.format = format;

                if (!parseCmapFormat2(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 2" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat2>(subtable));
                break;
            }
            case 4:
            {
                CmapFormat4 subtable;
                subtable.format = format;

                if (!parseCmapFormat4(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 4" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat4>(subtable));
                break;
            }
            case 6:
            {
                CmapFormat6 subtable;
                subtable.format = format;

                if (!parseCmapFormat6(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 6" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat6>(subtable));
                break;
            }
            case 8:
            {
                CmapFormat8 subtable;
                subtable.format = format;

                if (!parseCmapFormat8(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 8" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat8>(subtable));
                break;
            }
            case 10:
            {
                CmapFormat10 subtable;
                subtable.format = format;

                if (!parseCmapFormat10(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 10" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat10>(subtable));
                break;
            }
            case 12:
            {
                CmapFormat12 subtable;
                subtable.format = format;

                if (!parseCmapFormat12(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 12" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat12>(subtable));
                break;
            }
            case 14:
            {
                CmapFormat14 subtable;
                subtable.format = format;

                if (!parseCmapFormat14(subtableOffset + 2, subtable)) {
                    std::cout << "Failure at format 14" << std::endl;
                    return false;
                }

                cmap.subtables.push_back(std::make_unique<CmapFormat14>(subtable));
                break;
            }


            default:
                std::cout << "Unknown or unhandled format: " << format << std::endl;
                break;
            }


            currentOffset += 8; // Move to the next encoding record

            return true;
        }
    }

    bool TTFParser::parseMaxpTable(uint32_t offset) {
        if (fontData.size() < offset + 6) { // 6 bytes is the minimum size to get version and numGlyphs
            std::cerr << "Error: not enough data for 'maxp' table." << std::endl;
            return false;
        }

        // The version of the 'maxp' table (could be 0.5 or 1.0). 
        // We'll read it but won't use it for now.
        float version = fixedToFloat(swapEndian32(*(int32_t*)&fontData[offset]));
        offset += 4;

        numGlyphs = swapEndian16(*(uint16_t*)&fontData[offset]);

        return true;
    }

    bool TTFParser::parseOS2Table(uint32_t offset, OS2Table& os2) {
        // Base size check for version 0
        if (fontData.size() < offset + 78) {
            std::cerr << "Error: not enough data for 'OS/2' table." << std::endl;
            return false;
        }

        os2.version = swapEndian16(*(uint16_t*)&fontData[offset]);

        if (os2.version > 4) {
            std::cerr << "Error: Unsupported 'OS/2' table version." << std::endl;
            return false;
        }

        // Fields common for all versions
        os2.xAvgCharWidth = swapEndian16(*(int16_t*)&fontData[offset + 2]);
        os2.usWeightClass = swapEndian16(*(uint16_t*)&fontData[offset + 4]);
        os2.usWidthClass = swapEndian16(*(uint16_t*)&fontData[offset + 6]);
        os2.fsType = swapEndian16(*(uint16_t*)&fontData[offset + 8]);
        os2.ySubscriptXSize = swapEndian16(*(int16_t*)&fontData[offset + 10]);
        os2.ySubscriptYSize = swapEndian16(*(int16_t*)&fontData[offset + 12]);
        os2.ySubscriptXOffset = swapEndian16(*(int16_t*)&fontData[offset + 14]);
        os2.ySubscriptYOffset = swapEndian16(*(int16_t*)&fontData[offset + 16]);
        os2.ySuperscriptXSize = swapEndian16(*(int16_t*)&fontData[offset + 18]);
        os2.ySuperscriptYSize = swapEndian16(*(int16_t*)&fontData[offset + 20]);
        os2.ySuperscriptXOffset = swapEndian16(*(int16_t*)&fontData[offset + 22]);
        os2.ySuperscriptYOffset = swapEndian16(*(int16_t*)&fontData[offset + 24]);
        os2.yStrikeoutSize = swapEndian16(*(int16_t*)&fontData[offset + 26]);
        os2.yStrikeoutPosition = swapEndian16(*(int16_t*)&fontData[offset + 28]);
        os2.sFamilyClass = swapEndian16(*(int16_t*)&fontData[offset + 30]);
        std::memcpy(os2.panose, &fontData[offset + 32], 10);
        os2.ulUnicodeRange1 = swapEndian32(*(uint32_t*)&fontData[offset + 42]);
        os2.ulUnicodeRange2 = swapEndian32(*(uint32_t*)&fontData[offset + 46]);
        os2.ulUnicodeRange3 = swapEndian32(*(uint32_t*)&fontData[offset + 50]);
        os2.ulUnicodeRange4 = swapEndian32(*(uint32_t*)&fontData[offset + 54]);
        std::memcpy(os2.achVendID, &fontData[offset + 58], 4);
        os2.fsSelection = swapEndian16(*(uint16_t*)&fontData[offset + 62]);
        os2.usFirstCharIndex = swapEndian16(*(uint16_t*)&fontData[offset + 64]);
        os2.usLastCharIndex = swapEndian16(*(uint16_t*)&fontData[offset + 66]);
        os2.sTypoAscender = swapEndian16(*(int16_t*)&fontData[offset + 68]);
        os2.sTypoDescender = swapEndian16(*(int16_t*)&fontData[offset + 70]);
        os2.sTypoLineGap = swapEndian16(*(int16_t*)&fontData[offset + 72]);
        os2.usWinAscent = swapEndian16(*(uint16_t*)&fontData[offset + 74]);
        os2.usWinDescent = swapEndian16(*(uint16_t*)&fontData[offset + 76]);

        // Version 1 specific fields
        if (os2.version >= 1) {
            if (fontData.size() < offset + 80) {  // Check for two additional bytes for version 1
                std::cerr << "Error: not enough data for 'OS/2' table version 1." << std::endl;
                return false;
            }

            os2.usBreakChar = swapEndian16(*(uint16_t*)&fontData[offset + 78]);
            os2.usDefaultChar = swapEndian16(*(uint16_t*)&fontData[offset + 80]);
        }

        if (os2.version >= 2) {
            if (fontData.size() < offset + 102) {  // Check for version 2 additional bytes
                std::cerr << "Error: not enough data for 'OS/2' table version 2." << std::endl;
                return false;
            }

            os2.sCapHeight = swapEndian16(*(int16_t*)&fontData[offset + 88]);
            os2.usMaxContext = swapEndian16(*(uint16_t*)&fontData[offset + 90]);
            os2.ulCodePageRange1 = swapEndian32(*(uint32_t*)&fontData[offset + 94]);
            os2.sxHeight = swapEndian16(*(int16_t*)&fontData[offset + 96]);
            os2.ulCodePageRange2 = swapEndian32(*(uint32_t*)&fontData[offset + 98]);
        }

        if (os2.version >= 3) {
            if (fontData.size() < offset + 104) {  // Check for version 3 additional bytes
                std::cerr << "Error: not enough data for 'OS/2' table version 3." << std::endl;
                return false;
            }

            os2.usLowerOpticalPointSize = swapEndian16(*(uint16_t*)&fontData[offset + 100]);
            os2.usUpperOpticalPointSize = swapEndian16(*(uint16_t*)&fontData[offset + 102]);
        }

        return true;
    }

    bool TTFParser::parsePostTable(uint32_t offset, PostTable& post) {
        if (fontData.size() < offset + 32) { // Base size for the common fields.
            std::cerr << "Error: not enough data for 'post' table." << std::endl;
            return false;
        }

        post.format = readFixed(offset);
        post.italicAngle = readFixed(offset + 4);
        post.underlinePosition = swapEndian16(*(int16_t*)&fontData[offset + 8]);
        post.underlineThickness = swapEndian16(*(int16_t*)&fontData[offset + 10]);
        post.isFixedPitch = swapEndian32(*(uint32_t*)&fontData[offset + 12]);
        post.minMemType42 = swapEndian32(*(uint32_t*)&fontData[offset + 16]);
        post.maxMemType42 = swapEndian32(*(uint32_t*)&fontData[offset + 20]);
        post.minMemType1 = swapEndian32(*(uint32_t*)&fontData[offset + 24]);
        post.maxMemType1 = swapEndian32(*(uint32_t*)&fontData[offset + 28]);

        // Now, based on the format, we'll parse additional fields.
        if (post.format == 2.0) {
            // Parsing code for format 2.0
            post.numberOfGlyphs = swapEndian16(*(uint16_t*)&fontData[offset + 32]);

            if (fontData.size() < offset + 34 + 2 * post.numberOfGlyphs) {
                std::cerr << "Error: not enough data for 'post' table format 2.0." << std::endl;
                return false;
            }

            for (uint16_t i = 0; i < post.numberOfGlyphs; i++) {
                post.glyphNameIndex.push_back(swapEndian16(*(uint16_t*)&fontData[offset + 34 + i * 2]));
            }

            uint32_t nameOffset = offset + 34 + 2 * post.numberOfGlyphs;
            while (nameOffset < fontData.size()) {
                uint8_t nameLength = fontData[nameOffset];
                post.names.push_back(std::string((char*)&fontData[nameOffset + 1], nameLength));
                nameOffset += nameLength + 1;
            }
        }
        else if (post.format == 2.5) {
            // Parsing code for format 2.5
            post.numberOfGlyphs = swapEndian16(*(uint16_t*)&fontData[offset + 32]);

            if (fontData.size() < offset + 34 + post.numberOfGlyphs) {
                std::cerr << "Error: not enough data for 'post' table format 2.5." << std::endl;
                return false;
            }

            for (uint16_t i = 0; i < post.numberOfGlyphs; i++) {
                post.offset.push_back((int8_t)fontData[offset + 34 + i]);
            }
        }

        else if (post.format == 3.0) {
            // Format 3.0 has no additional data.
        }

        else {
            std::cerr << "Error: Unsupported 'post' table format." << std::endl;
            return false;
        }

        return true;
    }

    bool TTFParser::parseLocaTable(uint32_t locaOffset, LocaTable& loca) {
        if (locaOffset == 0) {
            std::cerr << "Error: 'loca' table not found." << std::endl;
            return false;
        }

        uint32_t offset = locaOffset;
        uint32_t previousOffset = 0;

        if (headTable.indexToLocFormat == 0) { // Short format
            for (size_t i = 0; i <= numGlyphs; ++i) {
                if (offset + 2 > fontData.size()) {
                    std::cerr << "Error: Unexpected end of data while parsing 'loca' table." << std::endl;
                    return false;
                }

                uint16_t shortOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
                uint32_t actualOffset = static_cast<uint32_t>(shortOffset) * 2;

                if (actualOffset < previousOffset) {
                    std::cerr << "Error: 'loca' table offsets are not in ascending order." << std::endl;
                    return false;
                }

                loca.offsets.push_back(actualOffset);
                previousOffset = actualOffset;
                offset += 2;
            }
        }

        else if (headTable.indexToLocFormat == 1) { // Long format
            for (size_t i = 0; i <= numGlyphs; ++i) {
                if (offset + 4 > fontData.size()) {
                    std::cerr << "Error: Unexpected end of data while parsing 'loca' table." << std::endl;
                    return false;
                }

                uint32_t longOffset = swapEndian32(*(uint32_t*)&fontData[offset]);

                if (longOffset < previousOffset) {
                    std::cerr << "Error: 'loca' table offsets are not in ascending order." << std::endl;
                    return false;
                }

                loca.offsets.push_back(longOffset);
                previousOffset = longOffset;
                offset += 4;
            }
        }
        else {
            std::cerr << "Error: Invalid 'indexToLocFormat' in 'head' table." << std::endl;
            return false;
        }

        // Ensure the last offset does not exceed the 'glyf' table length
        uint32_t glyfOffset = getTableOffset("glyf");
        uint32_t glyfLength = getTableLength("glyf");
        if (glyfOffset == 0 || glyfLength == 0) {
            std::cerr << "Error: 'glyf' table not found or has zero length." << std::endl;
            return false;
        }

        if (loca.offsets.back() > glyfLength) {
            std::cerr << "Error: Last offset in 'loca' table exceeds 'glyf' table length." << std::endl;
            return false;
        }

        return true;
    }

    bool TTFParser::parseKernTable(uint32_t offset, KernTable& table) {
        if (offset + 4 > fontData.size()) {
            return false;
        }

        table.version = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
        uint16_t nTables = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;

        for (uint16_t i = 0; i < nTables; ++i) {
            KernSubtable subtable;

            if (offset + 6 > fontData.size()) {
                return false;
            }

            subtable.version = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
            subtable.length = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
            subtable.coverage = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;

            if ((subtable.coverage & 0xFF) == 0) { // Format 0
                if (offset + 8 > fontData.size()) {
                    return false;
                }

                uint16_t nPairs = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
                offset += 6;  // Skip searchRange, entrySelector, rangeShift

                for (uint16_t j = 0; j < nPairs; ++j) {
                    if (offset + 6 > fontData.size()) {
                        return false;
                    }

                    KerningPair pair;
                    pair.left = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
                    pair.right = swapEndian16(*(uint16_t*)&fontData[offset]); offset += 2;
                    pair.value = swapEndian16(*(int16_t*)&fontData[offset]); offset += 2;

                    subtable.kerningPairs.push_back(pair);
                }
            }
            else {
                // Skip over the subtable if the format is not 0
                offset += subtable.length - 6;  // Subtract the 6 bytes we already read
            }

            table.subtables.push_back(subtable);
        }

        return true;
    }

    bool TTFParser::parseGPOSHeader(uint32_t offset, GPOSHeader& header) {
        // Ensure we have enough data for the header
        if (offset + 12 > fontData.size()) {
            std::cerr << "Failed to read GPOS header: insufficient data." << std::endl;
            return false;
        }

        // Read version
        header.version = swapEndian32(*(uint32_t*)&fontData[offset]);
        offset += 4;

        // Read ScriptList offset
        header.scriptListOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        // Read FeatureList offset
        header.featureListOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        // Read LookupList offset
        header.lookupListOffset = swapEndian16(*(uint16_t*)&fontData[offset]);

        std::cout << "Parsed GPOS Header: Version " << header.version << std::endl;

        return true;
    }

    bool TTFParser::parseLookupList(uint32_t offset, std::vector<LookupTable>& lookups) {
        // Ensure we have enough data for lookupCount
        if (offset + 2 > fontData.size()) {
            std::cerr << "Failed to read lookupCount: insufficient data." << std::endl;
            return false;
        }

        uint16_t lookupCount = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        for (uint16_t i = 0; i < lookupCount; ++i) {
            LookupTable lookup;

            // Ensure we have enough data for lookup table
            if (offset + 6 > fontData.size()) {
                std::cerr << "Failed to read LookupTable: insufficient data." << std::endl;
                return false;
            }

            lookup.lookupType = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            lookup.lookupFlag = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            lookup.subTableCount = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            // Read offsets for each subtable
            for (uint16_t j = 0; j < lookup.subTableCount; ++j) {
                if (offset + 2 > fontData.size()) {
                    std::cerr << "Failed to read subTableOffset: insufficient data." << std::endl;
                    return false;
                }
                uint16_t subTableOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
                lookup.subTableOffsets.push_back(subTableOffset);
                offset += 2;
            }

            lookups.push_back(lookup);
        }

        return true;
    }

    bool TTFParser::parseScriptList(uint32_t offset, std::vector<ScriptRecord>& scripts) {
        // Ensure we have enough data for scriptCount
        if (offset + 2 > fontData.size()) {
            std::cerr << "Failed to read scriptCount: insufficient data." << std::endl;
            return false;
        }

        uint16_t scriptCount = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        for (uint16_t i = 0; i < scriptCount; ++i) {
            ScriptRecord script;

            // Ensure we have enough data for ScriptRecord
            if (offset + 6 > fontData.size()) {
                std::cerr << "Failed to read ScriptRecord: insufficient data." << std::endl;
                return false;
            }

            script.scriptTag = swapEndian32(*(uint32_t*)&fontData[offset]);
            offset += 4;

            script.scriptOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            scripts.push_back(script);
        }

        return true;
    }

    bool TTFParser::parseScriptTable(uint32_t offset, ScriptTable& scriptTable) {
        if (offset + 4 > fontData.size()) {
            std::cerr << "Failed to read ScriptTable: insufficient data." << std::endl;
            return false;
        }

        scriptTable.defaultLangSys = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        scriptTable.langSysCount = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        for (uint16_t i = 0; i < scriptTable.langSysCount; ++i) {
            if (offset + 6 > fontData.size()) {
                std::cerr << "Failed to read LangSysRecord: insufficient data." << std::endl;
                return false;
            }

            LangSysRecord langSysRecord;
            langSysRecord.langSysTag = swapEndian32(*(uint32_t*)&fontData[offset]);
            offset += 4;

            langSysRecord.langSysOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            scriptTable.langSystems.push_back(langSysRecord);
        }

        return true;
    }

    bool TTFParser::parseFeatureList(uint32_t offset, std::vector<FeatureRecord>& features) {
        // Ensure we have enough data for featureCount
        if (offset + 2 > fontData.size()) {
            std::cerr << "Failed to read featureCount: insufficient data." << std::endl;
            return false;
        }

        uint16_t featureCount = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        for (uint16_t i = 0; i < featureCount; ++i) {
            FeatureRecord feature;

            // Ensure we have enough data for FeatureRecord
            if (offset + 6 > fontData.size()) {
                std::cerr << "Failed to read FeatureRecord: insufficient data." << std::endl;
                return false;
            }

            feature.featureTag = swapEndian32(*(uint32_t*)&fontData[offset]);
            offset += 4;

            feature.featureOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
            offset += 2;

            features.push_back(feature);
        }

        return true;
    }

    bool TTFParser::parseValueRecord(uint32_t& offset, uint16_t valueFormat, ValueRecord& value) {
        if (valueFormat & 0x0001) { // XPlacement
            if (offset + 2 > fontData.size()) return false;
            value.xPlacement = swapEndian16(*(int16_t*)&fontData[offset]);
            offset += 2;
        }
        if (valueFormat & 0x0002) { // YPlacement
            if (offset + 2 > fontData.size()) return false;
            value.yPlacement = swapEndian16(*(int16_t*)&fontData[offset]);
            offset += 2;
        }
        if (valueFormat & 0x0004) { // XAdvance
            if (offset + 2 > fontData.size()) return false;
            value.xAdvance = swapEndian16(*(int16_t*)&fontData[offset]);
            offset += 2;
        }
        if (valueFormat & 0x0008) { // YAdvance
            if (offset + 2 > fontData.size()) return false;
            value.yAdvance = swapEndian16(*(int16_t*)&fontData[offset]);
            offset += 2;
        }
        // ... Continue this pattern for other bits in valueFormat
        return true;
    }

    uint16_t TTFParser::getValueRecordSize(uint16_t valueFormat) {
        uint16_t size = 0;
        if (valueFormat & 0x0001) size += 2; // XPlacement
        if (valueFormat & 0x0002) size += 2; // YPlacement
        if (valueFormat & 0x0004) size += 2; // XAdvance
        if (valueFormat & 0x0008) size += 2; // YAdvance

        // ... Continue this pattern for other bits in valueFormat
        return size;
    }

    uint16_t TTFParser::getCoverageGlyphCount(uint32_t offset) {
        if (offset + 2 > fontData.size()) {
            std::cerr << "Failed to read Coverage format: insufficient data." << std::endl;
            return 0;
        }

        uint16_t format = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;  // Increment offset after reading format

        if (format == 1) {
            if (offset + 2 > fontData.size()) {  // Only need 2 bytes for glyph count
                std::cerr << "Failed to read Coverage format 1 glyph count: insufficient data." << std::endl;
                return 0;
            }

            uint16_t glyphCount = swapEndian16(*(uint16_t*)&fontData[offset]);

            return glyphCount;
        }

        else if (format == 2) {
            if (offset + 2 > fontData.size()) {  // Only need 2 bytes for range count
                std::cerr << "Failed to read Coverage format 2 range count: insufficient data." << std::endl;
                return 0;
            }
            uint16_t rangeCount = swapEndian16(*(uint16_t*)&fontData[offset]);

            return rangeCount;  // Note: This is not the total glyph count but the range count.
        }

        else {
            std::cerr << "Unknown Coverage format: " << format << " at offset: " << offset - 2 << std::endl;
            
            std::cerr << "Data around unknown format (" << format << ") at offset " << offset << ": ";
            for (int i = -10; i <= 10; i++) {
                std::cerr << std::hex << (int)fontData[offset + i] << " ";
            }

            std::cerr << std::endl;

            return 0;
        }
    }

    bool TTFParser::parseSingleAdjustmentSubtable(uint32_t offset, SingleAdjustmentSubtable& subtable) {
        // Ensure we have enough data for the basic fields
        if (offset + 6 > fontData.size()) {
            std::cerr << "Failed to read Single Adjustment Subtable: insufficient data." << std::endl;
            return false;
        }

        subtable.format = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        subtable.coverageOffset = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        subtable.valueFormat = swapEndian16(*(uint16_t*)&fontData[offset]);
        offset += 2;

        if (subtable.format == 1) {
            if (!parseValueRecord(offset, subtable.valueFormat, subtable.value)) {
                return false;
            }
        }
        else if (subtable.format == 2) {
            uint16_t glyphCount = getCoverageGlyphCount(offset + subtable.coverageOffset - 4);  // Adjust for the relative coverageOffset
            for (uint16_t i = 0; i < glyphCount; ++i) {
                ValueRecord value;
                if (!parseValueRecord(offset, subtable.valueFormat, value)) {
                    return false;
                }
                subtable.values.push_back(value);
                offset += getValueRecordSize(subtable.valueFormat);
            }
        }
        else {
            std::cerr << "Unknown Single Adjustment Subtable format." << std::endl;
            return false;
        }

        return true;
    }

}// namespace TTFParser