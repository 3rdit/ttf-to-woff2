#include "TTFParser.hpp"
#include <iostream>

int main() {
    const std::string ttfFilePath = "C:\\Users\\azulx\\Desktop\\Source Files\\Software\\tff-to-woff2\\Debug\\ArimaMadura.ttf"; // Replace with your TTF file path

    TTFParser::TTFParser parser;
    if (!parser.loadFromFile(ttfFilePath)) {
        std::cerr << "Failed to load TTF file: " << ttfFilePath << std::endl;
        return 1;
    }

    const auto& tableEntries = parser.getTableDirectoryEntries();
    for (const auto& entry : tableEntries) {
        char tagStr[5];
        std::memcpy(tagStr, &entry.tag, 4);
        tagStr[4] = '\0';
        std::cout << "Found table: " << tagStr << std::endl;
    }

    std::cout << "Successfully loaded TTF file: " << ttfFilePath << std::endl;

    const auto& headData = parser.getTableData("head");
    if (!headData.empty()) {
        uint32_t headOffset = parser.getTableOffset("head");

        if (headOffset == 0) {
            std::cerr << "Failed to find the 'head' table's offset." << std::endl;
            return 1;
        }

        if (!parser.parseHeadTable(headOffset)) {
            std::cerr << "Failed to parse 'head' table." << std::endl;
            return 1;
        }

        else {
            std::cout << "Parsed 'head' table successfully." << std::endl;
            std::cout << "Font version: " << TTFParser::headTable.fontRevision << std::endl;
            std::cout << "Created Date: " << TTFParser::headTable.created << std::endl;
            std::cout << "Modified Date: " << TTFParser::headTable.modified << std::endl;
        }
    }
    else {
        std::cout << "No 'head' table found in the font." << std::endl;
    }

    std::cout << std::endl;

    uint32_t gposOffset = parser.getTableOffset("GPOS");

    if (gposOffset == 0) {
        std::cerr << "Failed to locate 'GPOS' table." << std::endl;
        return 1;
    }

    TTFParser::GPOSHeader gposHeader;
    if (!parser.parseGPOSHeader(gposOffset, gposHeader)) {
        std::cerr << "Failed to parse 'GPOS' table header." << std::endl;
        return 1;
    }

    std::vector<TTFParser::ScriptRecord> scripts;

    if (!parser.parseScriptList(gposOffset + gposHeader.scriptListOffset, scripts)) {
        std::cerr << "Failed to parse the 'GPOS' table Script List." << std::endl;
        return 1;
    }

    // If we successfully parsed the Script List, print the details of each ScriptRecord and their ScriptTables
    for (size_t i = 0; i < scripts.size(); ++i) {
        const auto& script = scripts[i];
        std::cout << "Script " << i + 1 << ":\n";
        std::cout << "\tScript Tag: " << script.scriptTag << "\n"; // This might need formatting
        std::cout << "\tScript Offset: " << script.scriptOffset << "\n";

        TTFParser::ScriptTable scriptTable;
        if (!parser.parseScriptTable(gposOffset + gposHeader.scriptListOffset + script.scriptOffset, scriptTable)) {
            std::cerr << "Failed to parse the 'GPOS' table Script Table for script " << i + 1 << "." << std::endl;
            continue; // Move on to the next script
        }

        std::cout << "\tDefault Language System Offset: " << scriptTable.defaultLangSys << "\n";
        std::cout << "\tLanguage System Count: " << scriptTable.langSysCount << "\n";
        for (size_t j = 0; j < scriptTable.langSystems.size(); ++j) {
            const auto& langSys = scriptTable.langSystems[j];
            std::cout << "\t\tLanguage System " << j + 1 << " Tag: " << langSys.langSysTag << "\n"; // This might need formatting
            std::cout << "\t\tLanguage System " << j + 1 << " Offset: " << langSys.langSysOffset << "\n";
        }
        std::cout << std::endl;
    }

    std::vector<TTFParser::FeatureRecord> features;

    if (!parser.parseFeatureList(gposOffset + gposHeader.featureListOffset, features)) {
        std::cerr << "Failed to parse the 'GPOS' table Feature List." << std::endl;
        return 1;
    }

    // If we successfully parsed the Feature List, print the details of each FeatureRecord
    for (size_t i = 0; i < features.size(); ++i) {
        const auto& feature = features[i];
        std::cout << "Feature " << i + 1 << ":\n";
        std::cout << "\tFeature Tag: " << feature.featureTag << "\n"; // This might need formatting
        std::cout << "\tFeature Offset: " << feature.featureOffset << "\n";
        std::cout << std::endl;
    }

    system("pause");

    std::vector<TTFParser::LookupTable> lookups;

    if (!parser.parseLookupList(gposOffset + gposHeader.lookupListOffset, lookups)) {
        std::cerr << "Failed to parse the 'GPOS' table Lookup List." << std::endl;
        return 1;
    }

    // If we successfully parsed the Lookup List, print the details of each LookupTable
    for (size_t i = 0; i < lookups.size(); ++i) {
        const auto& lookup = lookups[i];
        std::cout << "Lookup " << i + 1 << ":\n";
        std::cout << "\tLookup Type: " << lookup.lookupType << "\n";
        std::cout << "\tLookup Flag: " << lookup.lookupFlag << "\n";
        std::cout << "\tSubTable Count: " << lookup.subTableCount << "\n";
        for (size_t j = 0; j < lookup.subTableOffsets.size(); ++j) {
            std::cout << "\tSubTable " << j + 1 << " Offset: " << lookup.subTableOffsets[j] << "\n";
        }
        std::cout << std::endl;
    }

    for (const auto& lookup : lookups) {
        std::cout << "Testing getCoverageGlyphCount for Lookup Type: " << lookup.lookupType << "\n";
        for (const auto& subTableOffset : lookup.subTableOffsets) {
            uint32_t currentOffset = gposOffset + subTableOffset;

            // We assume that the Coverage table is at the start of the subtable for simplicity.
            // This assumption might not always be the case, depending on the lookupType.
            int glyphCount = parser.getCoverageGlyphCount(currentOffset);

            if (glyphCount < 0) {
                std::cerr << "Failed to get glyph count for subtable at offset: " << subTableOffset
                    << ". Error code: " << glyphCount << std::endl;
            }
            else {
                std::cout << "Glyph count in Coverage table at subtable offset " << subTableOffset << ": " << glyphCount << "\n";
            }
        }
        std::cout << std::endl;
    }


    system("pause");

    TTFParser::KernTable kern;
    uint32_t kernOffset = parser.getTableOffset("kern");
    if (kernOffset == 0) {
        std::cerr << "Failed to locate 'kern' table." << std::endl;
        return 1;
    }

    if (!parser.parseKernTable(kernOffset, kern)) {
        std::cerr << "Failed to parse 'kern' table." << std::endl;
        return 1;
    }

    std::cout << "Parsed 'kern' table successfully." << std::endl;
    std::cout << "Number of subtables: " << kern.subtables.size() << std::endl;
    for (const auto& subtable : kern.subtables) {
        std::cout << "Subtable with " << subtable.kerningPairs.size() << " kerning pairs:" << std::endl;
        for (const auto& pair : subtable.kerningPairs) {
            std::cout << "Left Glyph ID: " << pair.left << ", Right Glyph ID: " << pair.right << ", Value: " << pair.value << std::endl;
        }
    }

    system("pause");

    // Parsing the 'post' table
    TTFParser::PostTable post;
    uint32_t postOffset = parser.getTableOffset("post");
    if (postOffset == 0) {
        std::cerr << "Failed to locate 'post' table." << std::endl;
        return 1;
    }

    if (!parser.parsePostTable(postOffset, post)) {
        std::cerr << "Failed to parse 'post' table." << std::endl;
        return 1;
    }

    std::cout << "Parsed 'post' table successfully." << std::endl;
    std::cout << "Format: " << post.format << std::endl;
    std::cout << "Italic Angle: " << post.italicAngle << std::endl;
    std::cout << "Underline Position: " << post.underlinePosition << std::endl;
    std::cout << "Underline Thickness: " << post.underlineThickness << std::endl;
    std::cout << "Is Fixed Pitch: " << post.isFixedPitch << std::endl;
    std::cout << "Minimum Memory Type 42: " << post.minMemType42 << std::endl;
    std::cout << "Maximum Memory Type 42: " << post.maxMemType42 << std::endl;
    std::cout << "Minimum Memory Type 1: " << post.minMemType1 << std::endl;
    std::cout << "Maximum Memory Type 1: " << post.maxMemType1 << std::endl;

    if (post.format == 2.0) {
        std::cout << "Number of Glyphs: " << post.numberOfGlyphs << std::endl;
        for (size_t i = 0; i < post.glyphNameIndex.size(); ++i) {
            std::cout << "Glyph " << i << " Name Index: " << post.glyphNameIndex[i] << std::endl;
        }
        for (size_t i = 0; i < post.names.size(); ++i) {
            std::cout << "Additional Name " << i << ": " << post.names[i] << std::endl;
        }
    }

    system("pause");

    TTFParser::OS2Table os2;
    uint32_t os2Offset = parser.getTableOffset("OS/2");
    if (os2Offset == 0) {
        std::cerr << "Failed to locate 'OS/2' table." << std::endl;
        return 1;
    }

    if (!parser.parseOS2Table(os2Offset, os2)) {
        std::cerr << "Failed to parse 'OS/2' table." << std::endl;
        return 1;
    }

    std::cout << "Parsed 'OS/2' table successfully." << std::endl;
    std::cout << "Version: " << os2.version << std::endl;
    std::cout << "xAvgCharWidth: " << os2.xAvgCharWidth << std::endl;
    std::cout << "usWeightClass: " << os2.usWeightClass << std::endl;
    std::cout << "usWidthClass: " << os2.usWidthClass << std::endl;
    std::cout << "fsType: " << os2.fsType << std::endl;
    std::cout << "ySubscriptXSize: " << os2.ySubscriptXSize << std::endl;
    std::cout << "ySubscriptYSize: " << os2.ySubscriptYSize << std::endl;
    std::cout << "ySubscriptXOffset: " << os2.ySubscriptXOffset << std::endl;
    std::cout << "ySubscriptYOffset: " << os2.ySubscriptYOffset << std::endl;
    std::cout << "ySuperscriptXSize: " << os2.ySuperscriptXSize << std::endl;
    std::cout << "ySuperscriptYSize: " << os2.ySuperscriptYSize << std::endl;
    std::cout << "ySuperscriptXOffset: " << os2.ySuperscriptXOffset << std::endl;
    std::cout << "ySuperscriptYOffset: " << os2.ySuperscriptYOffset << std::endl;
    std::cout << "yStrikeoutSize: " << os2.yStrikeoutSize << std::endl;
    std::cout << "yStrikeoutPosition: " << os2.yStrikeoutPosition << std::endl;
    std::cout << "sFamilyClass: " << os2.sFamilyClass << std::endl;

    std::cout << "panose: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << (int)os2.panose[i] << " ";
    }

    std::cout << std::endl;
    std::cout << "ulUnicodeRange1: " << os2.ulUnicodeRange1 << std::endl;
    std::cout << "ulUnicodeRange2: " << os2.ulUnicodeRange2 << std::endl;
    std::cout << "ulUnicodeRange3: " << os2.ulUnicodeRange3 << std::endl;
    std::cout << "ulUnicodeRange4: " << os2.ulUnicodeRange4 << std::endl;
    std::cout << "achVendID: " << std::string(os2.achVendID, 4) << std::endl; // Display the vendor ID as a string
    std::cout << "fsSelection: " << os2.fsSelection << std::endl;
    std::cout << "usFirstCharIndex: " << os2.usFirstCharIndex << std::endl;
    std::cout << "usLastCharIndex: " << os2.usLastCharIndex << std::endl;
    std::cout << "sTypoAscender: " << os2.sTypoAscender << std::endl;
    std::cout << "sTypoDescender: " << os2.sTypoDescender << std::endl;
    std::cout << "sTypoLineGap: " << os2.sTypoLineGap << std::endl;
    std::cout << "usWinAscent: " << os2.usWinAscent << std::endl;
    std::cout << "usWinDescent: " << os2.usWinDescent << std::endl;

    if (os2.version >= 1) {
        std::cout << "usBreakChar: " << os2.usBreakChar << std::endl;
        std::cout << "usDefaultChar: " << os2.usDefaultChar << std::endl;
    }

    if (os2.version >= 2) {
        std::cout << "sCapHeight: " << os2.sCapHeight << std::endl;
        std::cout << "usMaxContext: " << os2.usMaxContext << std::endl;
        std::cout << "ulCodePageRange1: " << os2.ulCodePageRange1 << std::endl;
        std::cout << "sxHeight: " << os2.sxHeight << std::endl;
        std::cout << "ulCodePageRange2: " << os2.ulCodePageRange2 << std::endl;
    }

    if (os2.version >= 3) {
        std::cout << "usLowerOpticalPointSize: " << os2.usLowerOpticalPointSize << std::endl;
        std::cout << "usUpperOpticalPointSize: " << os2.usUpperOpticalPointSize << std::endl;
        // sCapHeight, usDefaultChar, usBreakChar, and usMaxContext are already printed in the previous blocks
    }

    system("pause");


    // Test the 'name' table parsing
    TTFParser::NameTable nameTable;
    uint32_t nameTableOffset = parser.getTableOffset("name");
    if (nameTableOffset == 0) {
        std::cerr << "Failed to find 'name' table." << std::endl;
        return 1;
    }

    if (!parser.parseNameTable(nameTableOffset, nameTable)) {
        std::cerr << "Failed to parse 'name' table." << std::endl;
        return 1;
    }

    // Print out the parsed 'name' table details
    std::cout << "\nName Table:" << std::endl;
    std::cout << "Format: " << nameTable.format << std::endl;
    std::cout << "Count: " << nameTable.count << std::endl;

    for (const auto& record : nameTable.nameRecords) {
        std::cout << "----------------------------------" << std::endl;
        std::cout << "Platform ID: " << record.platformID << std::endl;
        std::cout << "Encoding ID: " << record.encodingID << std::endl;
        std::cout << "Language ID: " << record.languageID << std::endl;
        std::cout << "Name ID: " << record.nameID << std::endl;
        std::cout << "Length: " << record.length << std::endl;
        std::cout << "Offset: " << record.offset << std::endl;
        std::cout << "Name String: " << record.nameString << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;



    uint32_t maxpOffset = parser.getTableOffset("maxp");
    if (maxpOffset == 0) {
        std::cerr << "Failed to locate 'maxp' table." << std::endl;
        return 1;
    }

    if (!parser.parseMaxpTable(maxpOffset)) {
        std::cerr << "Failed to parse 'maxp' table." << std::endl;
        return 1;
    }

        // Parsing the 'hhea' table
    TTFParser::HheaTable hhea;
    uint32_t hheaOffset = parser.getTableOffset("hhea");
    if (hheaOffset == 0) {
        std::cerr << "Failed to locate 'hhea' table." << std::endl;
        return 1;
    }

    if (!parser.parseHheaTable(hheaOffset, hhea)) {
        std::cerr << "Failed to parse 'hhea' table." << std::endl;
        return 1;
    }

    std::cout << "Parsed 'hhea' table successfully." << std::endl;
    std::cout << "Ascent: " << hhea.ascent << std::endl;
    std::cout << "Descent: " << hhea.descent << std::endl;
    std::cout << "Line Gap: " << hhea.lineGap << std::endl;
    std::cout << "Max Advance Width: " << hhea.advanceWidthMax << std::endl;
    std::cout << "Number of Long Horizontal Metrics: " << hhea.numOfLongHorMetrics << std::endl;

    std::vector<TTFParser::GlyphMetrics> metrics;
    uint32_t hmtxOffset = parser.getTableOffset("hmtx");
    if (hmtxOffset == 0) {
        std::cerr << "Failed to locate 'hmtx' table." << std::endl;
        return 1;
    }

    if (!parser.parseHmtxTable(hmtxOffset, hhea.numOfLongHorMetrics, metrics)) {
        std::cerr << "Failed to parse 'hmtx' table." << std::endl;
        return 1;
    }

    std::cout << "Parsed 'hmtx' table successfully." << std::endl;

    // Let's print the advance width and left side bearing of the first few glyphs for demonstration:
    int numMetricsToDisplay = std::min(10, static_cast<int>(metrics.size())); // Displaying up to 10 metrics or less if there are fewer than 10
    for (int i = 0; i < numMetricsToDisplay; ++i) {
        std::cout << "Glyph " << i << ": Advance Width = " << metrics[i].advanceWidth
            << ", Left Side Bearing = " << metrics[i].lsb << std::endl;
    }

    // Test the cmap table parsing
    TTFParser::CmapTable cmap;
    const auto& cmapData = parser.getTableData("cmap");
    if (!cmapData.empty()) {
        uint32_t cmapOffset = parser.getTableOffset("cmap");

        if (cmapOffset == 0) {
            std::cerr << "Failed to find the 'cmap' table's offset." << std::endl;
            return 1;
        }

        if (!parser.parseCmapTable(cmapOffset, cmap)) {
            std::cerr << "Failed to parse 'cmap' table." << std::endl;
            return 1;
        }

        else {
            std::cout << "Parsed 'cmap' table successfully." << std::endl;
            std::cout << "Number of subtables: " << cmap.subtables.size() << std::endl;
            for (const auto& subtable : cmap.subtables) {
                std::cout << "Subtable format: " << subtable->format << std::endl;
                // You can add more detailed printouts for each subtable format if needed
            }
        }
    }
    else {
        std::cout << "No 'cmap' table found in the font." << std::endl;
    }


    // Parsing the 'loca' table
    TTFParser::LocaTable loca;
    uint32_t locaOffset = parser.getTableOffset("loca");
    if (locaOffset == 0) {
        std::cerr << "Failed to locate 'loca' table." << std::endl;
        return 1;
    }

    if (!parser.parseLocaTable(locaOffset, loca)) {
        std::cerr << "Failed to parse 'loca' table." << std::endl;
        return 1;
    }

    std::cout << "Parsed 'loca' table successfully." << std::endl;

    // Displaying up to the first 10 offsets or less if there are fewer than 10.
    int numOffsetsToDisplay = std::min(10, static_cast<int>(loca.offsets.size()));
    for (int i = 0; i < numOffsetsToDisplay; ++i) {
        std::cout << "Glyph " << i << " Offset: " << loca.offsets[i] << std::endl;
    }


    return 0;
}
