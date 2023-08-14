#ifndef TTF_PARSER_HPP
#define TTF_PARSER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <vector>

// Define necessary structures for the TTF format
// See https://docs.microsoft.com/en-us/typography/opentype/spec/otff for more information
namespace TTFParser {    

// The ValueRecord struct represents positioning adjustments for a glyph.
    struct ValueRecord {
        int16_t xPlacement;  // Horizontal adjustment for glyph placement.
        int16_t yPlacement;  // Vertical adjustment for glyph placement.
        int16_t xAdvance;    // Horizontal adjustment for glyph advance.
        int16_t yAdvance;    // Vertical adjustment for glyph advance.
        // ... There are more potential fields
    };

    // The SingleAdjustmentSubtable struct represents the single adjustment subtable within GPOS.
    struct SingleAdjustmentSubtable {
        uint16_t format;           // Format of the subtable (1 or 2).
        uint16_t coverageOffset;   // Offset to the coverage table.
        uint16_t valueFormat;      // Format of the value record.
        ValueRecord value;         // Value record for Format 1.
        std::vector<ValueRecord> values; // Value records for Format 2.
    };

    // The GPOSHeader struct represents the header of the Glyph Positioning Table (GPOS).
    struct GPOSHeader {
        uint32_t version;          // Version of the GPOS table.
        uint16_t scriptListOffset; // Offset to the ScriptList table.
        uint16_t featureListOffset;// Offset to the FeatureList table.
        uint16_t lookupListOffset; // Offset to the LookupList table.
    };

    // The LookupTable struct represents a lookup table within GPOS or GSUB.
    struct LookupTable {
        uint16_t lookupType;         // Type of lookup (e.g., 1 for Single Adjustment).
        uint16_t lookupFlag;         // Flags for lookup processing.
        uint16_t subTableCount;      // Number of subtables.
        std::vector<uint16_t> subTableOffsets; // Offsets to the subtables.
    };

    // The SinglePos struct represents a single positioning subtable.
    struct SinglePos {
        uint16_t format;            // Format of the subtable.
        uint16_t coverageOffset;    // Offset to the coverage table.
        // ... other fields depending on format
    };

    // The ScriptRecord struct represents a script record in the ScriptList table.
    struct ScriptRecord {
        uint32_t scriptTag;         // Tag identifying the script.
        uint16_t scriptOffset;      // Offset to the Script table.
    };

    // The LangSysRecord struct represents a language system record in the Script table.
    struct LangSysRecord {
        uint32_t langSysTag;        // Tag identifying the language system.
        uint16_t langSysOffset;     // Offset to the LangSys table.
    };

    // The ScriptTable struct represents the Script table containing language systems.
    struct ScriptTable {
        uint16_t defaultLangSys;    // Offset to the default LangSys table.
        uint16_t langSysCount;      // Number of language systems.
        std::vector<LangSysRecord> langSystems; // Language system records.
    };

    // The FeatureRecord struct represents a feature record in the FeatureList table.
    struct FeatureRecord {
        uint32_t featureTag;        // Tag identifying the feature.
        uint16_t featureOffset;     // Offset to the Feature table.
    };

    // The FeatureTable struct represents the Feature table containing lookups.
    struct FeatureTable {
        uint16_t featureParams;     // Additional parameters (can be null).
        uint16_t lookupCount;       // Number of lookup list indices.
        std::vector<uint16_t> lookupListIndices; // Indices of lookups.
    };

    // The KerningPair struct represents a pair of glyphs and their kerning value.
    struct KerningPair {
        uint16_t left;              // Left glyph in the pair.
        uint16_t right;             // Right glyph in the pair.
        int16_t value;              // Kerning value for the pair.
    };

    // The KernSubtable struct represents a kerning subtable within the 'kern' table.
    struct KernSubtable {
        uint16_t version;           // Version of the subtable.
        uint16_t length;            // Length of the subtable in bytes.
        uint16_t coverage;          // Coverage format.
        std::vector<KerningPair> kerningPairs; // Kerning pairs in the subtable.
    };

    // The KernTable struct represents the kerning table ('kern') containing subtables.
    struct KernTable {
        uint16_t version;           // Version of the 'kern' table.
        std::vector<KernSubtable> subtables; // Kerning subtables.
    };

    // The LocaTable struct represents the index to location table ('loca').
    struct LocaTable {
        std::vector<uint32_t> offsets; // Offsets to glyph data (could be a vector of uint16_t depending on format).
    };

    // The PostTable struct represents the PostScript table ('post') containing PostScript-related data.
    struct PostTable {
        float format;              // Format of the table.
        float italicAngle;         // Italic angle in counter-clockwise degrees.
        int16_t underlinePosition; // Underline position.
        int16_t underlineThickness;// Underline thickness.
        uint32_t isFixedPitch;     // Whether the font is monospaced (fixed-pitch).
        uint32_t minMemType42;     // Minimum memory usage for Type 42 font.
        uint32_t maxMemType42;     // Maximum memory usage for Type 42 font.
        uint32_t minMemType1;      // Minimum memory usage for Type 1 font.
        uint32_t maxMemType1;      // Maximum memory usage for Type 1 font.
        uint16_t numberOfGlyphs;   // Number of glyphs (for format 2.0).
        std::vector<uint16_t> glyphNameIndex; // Glyph name indices (for format 2.0).
        std::vector<std::string> names; // Additional names (for format 2.0).
        std::vector<int8_t> offset;  // Offsets for format 2.5 (rarely used).
    };

    struct OS2Table {
        uint16_t version;          // Version of the OS/2 table.
        int16_t  xAvgCharWidth;    // Average width of characters in the font.
        uint16_t usWeightClass;    // Weight class (e.g., 400 for normal, 700 for bold).
        uint16_t usWidthClass;     // Width class (e.g., 5 for medium).
        uint16_t fsType;           // Type flags (e.g., embedding permissions).
        int16_t  ySubscriptXSize;  // Horizontal size for subscripts.
        int16_t  ySubscriptYSize;  // Vertical size for subscripts.
        int16_t  ySubscriptXOffset;// Horizontal offset for subscripts.
        int16_t  ySubscriptYOffset;// Vertical offset for subscripts.
        int16_t  ySuperscriptXSize;// Horizontal size for superscripts.
        int16_t  ySuperscriptYSize;// Vertical size for superscripts.
        int16_t  ySuperscriptXOffset; // Horizontal offset for superscripts.
        int16_t  ySuperscriptYOffset; // Vertical offset for superscripts.


        int16_t  yStrikeoutSize;    // Size of strikeout line.
        int16_t  yStrikeoutPosition;// Position of strikeout line relative to baseline.
        int16_t  sFamilyClass;      // Font family class and subclass.
        uint8_t  panose[10];        // PANOSE classification numbers.
        uint32_t ulUnicodeRange1;   // Unicode range bits (0-31).
        uint32_t ulUnicodeRange2;   // Unicode range bits (32-63).
        uint32_t ulUnicodeRange3;   // Unicode range bits (64-95).
        uint32_t ulUnicodeRange4;   // Unicode range bits (96-127).
        char     achVendID[4];      // Font vendor identification.
        uint16_t fsSelection;       // Font selection flags.
        uint16_t usFirstCharIndex;  // Unicode code point of the first character in the font.
        uint16_t usLastCharIndex;   // Unicode code point of the last character in the font.
        int16_t  sTypoAscender;     // Typographic ascender.
        int16_t  sTypoDescender;    // Typographic descender.
        int16_t  sTypoLineGap;      // Typographic line gap.
        uint16_t usWinAscent;       // Windows ascender metric.
        uint16_t usWinDescent;      // Windows descender metric.
        uint16_t usBreakChar;       // Breaking character (New for version 1).
        uint16_t usDefaultChar;     // Default character (New for version 1).
        int16_t sCapHeight;         // Height of capital letters.
        uint16_t usMaxContext;      // Maximum context length for contextual lookups.
        uint32_t ulCodePageRange1;  // Code page range bits (0-31).
        uint32_t ulCodePageRange2;  // Code page range bits (32-63).
        uint16_t sxHeight;          // Height of lowercase letters.
        uint16_t usLowerOpticalPointSize; // Lower optical size (Version 3 and above).
        uint16_t usUpperOpticalPointSize; // Upper optical size (Version 3 and above).
        // Additional fields for higher versions...
    };

    // The NameRecord struct represents an individual name record within the 'name' table.
    struct NameRecord {
        uint16_t platformID;      // Platform ID for the name.
        uint16_t encodingID;      // Encoding ID for the name.
        uint16_t languageID;      // Language ID for the name.
        uint16_t nameID;          // Name ID for the name.
        uint16_t length;          // Length of the name string in bytes.
        uint16_t offset;          // Offset to the name string within the 'name' table.
        std::string nameString;   // The actual string value.
    };

    // The NameTable struct represents the 'name' table containing font-related names.
    struct NameTable {
        uint16_t format;          // Format of the 'name' table.
        uint16_t count;           // Number of name records.
        uint16_t stringOffset;    // Offset to the beginning of the name string storage.
        std::vector<NameRecord> nameRecords; // Name records.
    };

    // The HheaTable struct represents the horizontal header table ('hhea') containing horizontal metrics.
    struct HheaTable {
        float version;            // Table version number (0.5 or 1.0 for TrueType).
        int16_t ascent;           // Distance from baseline of highest ascender.
        int16_t descent;          // Distance from baseline of lowest descender.
        int16_t lineGap;          // Typographic line gap.
        uint16_t advanceWidthMax; // Maximum advance width value in 'hmtx' table.
        int16_t minLeftSideBearing; // Minimum left sidebearing value in 'hmtx' table.
        int16_t minRightSideBearing; // Minimum right sidebearing value; calculated as Min(aw - lsb - (xMax - xMin)).
        int16_t xMaxExtent;       // Max(lsb + (xMax - xMin)).
        int16_t caretSlopeRise;   // Used to calculate the slope of the caret (rise/run), set to 1 for vertical caret.
        int16_t caretSlopeRun;    // 0 for vertical.
        int16_t caretOffset;      // Set value to 0 for non-slanted fonts.
        int16_t reserved[4];      // Set to 0.
        int16_t metricDataFormat; // 0 for current format.
        uint16_t numOfLongHorMetrics; // Number of hMetric entries in 'hmtx' table.
    };

    struct GlyphMetrics {
        uint16_t advanceWidth;    // Advance width for the glyph.
        int16_t lsb;              // Left sidebearing for the glyph.
    };

    // The HeadTable struct represents the font header table ('head') containing font-wide parameters.
    struct HeadTable {
        float version;            // Version of the 'head' table.
        float fontRevision;       // Font revision number.
        uint32_t checkSumAdjustment; // Checksum adjustment for the font file.
        uint32_t magicNumber;     // Magic number (should be 0x5F0F3CF5).
        uint16_t flags;           // Font flags.
        uint16_t unitsPerEm;      // Number of units per em.
        int64_t created;          // Creation date.
        int64_t modified;         // Modification date.
        int16_t xMin;             // Minimum x for all glyph bounding boxes.
        int16_t yMin;             // Minimum y for all glyph bounding boxes.
        int16_t xMax;             // Maximum x for all glyph bounding boxes.
        int16_t yMax;             // Maximum y for all glyph bounding boxes.
        uint16_t macStyle;        // Macintosh style attributes.
        uint16_t lowestRecPPEM;   // Smallest readable size in pixels per em.
        int16_t fontDirectionHint;// Direction hint for layout.
        int16_t indexToLocFormat; // Format of 'loca' table (0 or 1).
        int16_t glyphDataFormat;  // Glyph data format.
    };

    // The GroupFormat12 struct represents a group in the cmap format 12 subtable.
    struct GroupFormat12 {
        uint32_t startCharCode;   // Starting Unicode code point.
        uint32_t endCharCode;     // Ending Unicode code point.
        uint32_t startGlyphID;    // Starting glyph ID.
    };

    // The GroupFormat13 struct represents a group in the cmap format 13 subtable.
    struct GroupFormat13 {
        uint32_t startCharCode;   // Starting Unicode code point.
        uint32_t endCharCode;     // Ending Unicode code point.
        uint32_t glyphID;         // Glyph ID for all code points in the range.
    };

    struct CmapSubtable {
        uint16_t format;
        // ... other common members for all subtables
    };

    struct CmapFormat0 : public CmapSubtable {
        uint8_t glyphIdArray[256];
    };

    struct SubHeader {
        uint16_t firstCode;      // First valid low byte for this subHeader.
        uint16_t entryCount;     // Number of valid low bytes for this subHeader.
        int16_t idDelta;         // For the character code obtained by concatenating this byte with a valid low byte,
        // subtract this value from the resulting character code to get the glyph index.
        uint16_t idRangeOffset;  // Number of bytes past this byte to get the glyph index in the glyphIndexArray.
        // This should always be an even number. If idRangeOffset is 0, no glyphIndexArray is used
        // and idDelta is added directly to the character code to get the glyph index.
    };

    struct UVSMapping {
        uint32_t unicodeValue; // 24 bits
        uint16_t glyphID;
    };

    struct UnicodeValueRange {
        uint32_t startUnicodeValue; // 24 bits
        uint8_t additionalCount;
    };


    struct VarSelectorRecord {
        uint32_t varSelector; // 24 bits
        uint32_t defaultUVSOffset;
        uint32_t nonDefaultUVSOffset;
        std::vector<UnicodeValueRange> defaultUVSTable;  // If present
        std::vector<UVSMapping> nonDefaultUVSTable;      // If present
    };

    struct CmapFormat2 : public CmapSubtable {
        uint16_t format;             // This should always be 2.
        uint16_t length;             // This is the length in bytes of the subtable.
        uint16_t language;           // This field can be used to specify a language.
        uint8_t subHeaderKeys[256];  // Array that maps high bytes to subHeaders. Value is subHeader index multiplied by 8.
        std::vector<SubHeader> subHeaders;       // Vector of SubHeaders.
        std::vector<uint16_t> glyphIndexArray;   // Array of glyph indices.
    };

    struct CmapFormat4 : public CmapSubtable {
        uint16_t segCountX2;              // Total number of segments, multiplied by 2.
        uint16_t searchRange;             // The largest power of 2 less than or equal to segCountX2, multiplied by 2.
        uint16_t entrySelector;           // Log2 of the largest power of 2 less than or equal to segCountX2.
        uint16_t rangeShift;              // Difference between segCountX2 and searchRange.
        std::vector<uint16_t> endCount;   // Array of ending character codes for each segment. Ordered by increasing character codes.
        uint16_t reservedPad;             // Padding value. Should always be 0.
        std::vector<uint16_t> startCount; // Array of starting character codes for each segment. Ordered by corresponding endCount values.
        std::vector<uint16_t> idDelta;    // Array of delta values for computing glyph index. Corresponds to segments in startCount and endCount.
        std::vector<uint16_t> idRangeOffset; // Array of offsets within the glyph index array. Corresponds to segments in startCount and endCount.

        std::vector<uint16_t> glyphIndices; // Array of glyph indices for characters not present in any segment.
    };

    struct CmapFormat6 : public CmapSubtable {
        uint16_t format;       // This should always be 6.
        uint16_t length;       // Length in bytes of the subtable.
        uint16_t language;     // Can be used to specify a language.
        uint16_t firstCode;    // First character code covered.
        uint16_t entryCount;   // Number of character codes covered.
        std::vector<uint16_t> glyphIdArray;  // Array of glyph indices.
    };

    struct CmapFormat8 : public CmapSubtable {
        uint16_t format;       // This should always be 8.
        uint16_t reserved;     // Set to 0.
        uint32_t length;       // Length in bytes of the subtable.
        uint32_t language;     // Can be used to specify a language.
        uint8_t is32[8192];    // Bitfield to check if a Unicode value is 32-bit.
        uint32_t numGroups;    // Number of groups.
        // You'll need a structure for groups and then a vector of that structure here.
    };

    struct CmapFormat10 : public CmapSubtable {
        uint16_t format;       // This should always be 10.
        uint16_t reserved;     // Set to 0.
        uint32_t length;       // Length in bytes of the subtable.
        uint32_t language;     // Can be used to specify a language.
        uint32_t startCharCode;  // First character code covered.
        uint32_t numChars;      // Number of character codes covered.
        std::vector<uint16_t> glyphs;  // Array of glyph indices.
    };

    struct CmapFormat12 : public CmapSubtable {
        uint16_t format;       // This should always be 12.
        uint16_t reserved;     // Set to 0.
        uint32_t length;       // Length in bytes of the subtable.
        uint32_t language;     // Can be used to specify a language.
        uint32_t numGroups;    // Number of groups.
        std::vector<GroupFormat12> groups;  // Vector of groups.

        // You'll need a structure for groups and then a vector of that structure here.
    };

    struct CmapFormat13 : public CmapSubtable {
        uint16_t format;       // This should always be 13.
        uint16_t reserved;     // Set to 0.
        uint32_t length;       // Length in bytes of the subtable.
        uint32_t language;     // Can be used to specify a language.
        uint32_t numGroups;    // Number of groups.

        std::vector<GroupFormat13> groups;  // Vector of groups.
        // You'll need a structure for groups and then a vector of that structure here.
    };

    // Represents the cmap format 14 subtable, used for Unicode variation sequences.
    struct CmapFormat14 : public CmapSubtable {
        uint16_t format;                      // Format identifier (should be 14).
        uint32_t length;                      // Length of the subtable.
        uint32_t numVarSelectorRecords;       // Number of variation selector records.
        std::vector<VarSelectorRecord> varSelectors; // Variation selector records.
    };

    // Contains all the subtables for the character mapping (cmap) table.
    struct CmapTable {
        uint16_t version;                     // Version number of the table.
        uint16_t numTables;                   // Number of subtables.
        std::vector<std::unique_ptr<CmapSubtable>> subtables; // Subtables.
    };

    // Represents an axis record in the font variations ('fvar') table.
    struct AxisRecord {
        uint32_t axisTag;                     // 4-byte identifier for the axis.
        uint16_t axisNameID;                  // Name ID for entries in the 'name' table.
        uint16_t axisOrdering;                // Ordering for the axis.
        float axisMinValue;                   // Minimum value for the axis.
        float axisDefaultValue;               // Default value for the axis.
        float axisMaxValue;                   // Maximum value for the axis.
        uint16_t flags;                       // Axis flags.
        uint16_t axisValueCount;              // Number of axis values.
        uint32_t axisValueOffset;             // Offset to the axis values.
    };

    // Font variations ('fvar') table, which describes axes for font variation.
    struct FVarTable {
        uint32_t version;                     // Version number of the table.
        uint16_t axisArrayOffset;             // Offset to the axis array.
        uint16_t axisCount;                   // Number of axes.
        uint16_t axisSize;                    // Size of each axis record.
        std::vector<AxisRecord> axes;         // Axis records.
    };

    // Represents a point in a glyph outline.
    struct Point {
        int16_t x;                            // X-coordinate.
        int16_t y;                            // Y-coordinate.
        bool onCurve;                         // True if the point is on the curve, false if off.
    };

    // Represents a simple glyph.
    struct SimpleGlyph {
        int16_t numberOfContours;             // Number of contours in the glyph.
        std::vector<uint16_t> endPointOfContours; // Endpoints for each contour.
        uint16_t instructionLength;           // Length of the instruction set.
        std::vector<uint8_t> instructions;    // Instructions for rendering the glyph.
        std::vector<Point> points;            // Points in the glyph outline.
    };

    // Represents a component in a compound glyph.
    struct CompoundComponent {
        uint16_t glyphIndex;                  // Index of the simple glyph.
        int arg1, arg2;                       // Transformation arguments.
        // TODO: Add transformation matrices or scaling factors as needed.

        CompoundComponent(uint16_t idx) : glyphIndex(idx), arg1(0), arg2(0) {}
    };

    // Represents a compound glyph, composed of one or more simple glyphs.
    struct CompoundGlyph {
        std::vector<CompoundComponent> components; // Components of the compound glyph.

        // We can add more members here as needed, like transformation data or instructions.
    };

    // Represents an entry in the table directory.
    struct TableDirectoryEntry {
        uint32_t tag;                         // 4-byte identifier for the table.
        uint32_t checkSum;                    // Checksum for the table.
        uint32_t offset;                      // Offset to the table from the beginning of the font file.
        uint32_t length;                      // Length of the table.
    };

    // Represents the offset table at the beginning of a TrueType or OpenType font file.
    struct OffsetTable {
        uint32_t sfntVersion;                 // Version of the sfnt structure (0x00010000 for version 1.0).
        uint16_t numTables;                   // Number of tables in the font.
        uint16_t searchRange;                 // (Maximum power of 2 <= numTables) x 16.
        uint16_t entrySelector;               // Log2(maximum power of 2 <= numTables).
        uint16_t rangeShift;                  // NumTables x 16-searchRange.
        std::vector<TableDirectoryEntry> tableDirectoryEntries; // Table directory entries.
    };

    // Represents the head table, which contains global information about the font. (used super often, this just makes it easier)
    static HeadTable headTable;

    /**
    * @class TTFParser
    * @brief A parser for TrueType Font (TTF) files, providing functionality to read various tables.
    */
    class TTFParser {
    public:
        TTFParser() = default;
        ~TTFParser() = default;

        /**
         * @brief Loads a TTF font from a given file path.
         * @param filename Path to the TTF file.
         * @return true if the file was loaded successfully, false otherwise.
         */
        bool loadFromFile(const std::string& filename);

        /**
         * @brief Retrieves the binary data of a table identified by its tag.
         * @param tag  identifier for the table.
         * @return Vector of bytes representing the table data.
         */
        const std::vector<uint8_t>& getTableData(const std::string& tag) const;

        // Parsing functions for various tables:
        bool parseHeadTable(uint32_t offset);
        bool parseHheaTable(uint32_t offset, HheaTable& table);
        bool parseHmtxTable(uint32_t offset, uint16_t numOfLongHorMetrics, std::vector<GlyphMetrics>& metrics);
        bool parseNameTable(uint32_t offset, NameTable& table);
        bool parseOS2Table(uint32_t offset, OS2Table& os2);
        bool parsePostTable(uint32_t offset, PostTable& post);
        bool parseLocaTable(uint32_t locaOffset, LocaTable& loca);
        bool parseKernTable(uint32_t offset, KernTable& table);
        bool parseFVarTable(uint32_t offset, FVarTable& fvar);
        bool parseCmapTable(uint32_t offset, CmapTable& cmap);
        bool parseMaxpTable(uint32_t offset);
        // ... More parsing functions ...

        const std::map<std::string, std::vector<uint8_t>>& getTableDataMap() const {
            return tableData;
        }

        // Functions to swap endianness:
        inline uint16_t swapEndian16(uint16_t value) {
            return (value << 8) | (value >> 8);
        }
        inline uint32_t swapEndian32(uint32_t value) {
            return ((value << 24) |
                ((value & 0xFF00) << 8) |
                ((value & 0xFF0000) >> 8) |
                (value >> 24));
        }
        int64_t swapEndian64(int64_t value) {
            value = ((value << 56) & 0xFF00000000000000) |
                ((value << 40) & 0x00FF000000000000) |
                ((value << 24) & 0x0000FF0000000000) |
                ((value << 8) & 0x000000FF00000000) |
                ((value >> 8) & 0x00000000FF000000) |
                ((value >> 24) & 0x0000000000FF0000) |
                ((value >> 40) & 0x000000000000FF00) |
                ((value >> 56) & 0x00000000000000FF);

            return value;
        }

        // Functions to convert fixed-point numbers:
        inline float f2Dot14ToFloat(int16_t value) {
            return static_cast<float>(value) / (1 << 14);
        }

        inline float fixedToFloat(int32_t value) {
            return (float)value / (1 << 16);
        }

        float readFixed(uint32_t offset) {
            int16_t intPart = swapEndian16(*(int16_t*)&fontData[offset]);
            uint16_t fracPart = swapEndian16(*(uint16_t*)&fontData[offset + 2]);
            return intPart + (fracPart / 65536.0f);
        }

        bool parseGlyph(uint32_t glyphOffset, SimpleGlyph& glyph);

        bool parseCompoundGlyph(uint32_t& offset, CompoundGlyph& glyph);

        /**
        * @brief Gets the entire binary data of the TTF file.
        * @return Vector of bytes representing the TTF file.
        */
        const std::vector<uint8_t>& getFontData() const {
            return fontData;
        }

        /*
        * @brief Gets the size of the binary data of the TTF file.
        * @return Size of the TTF file in bytes.
        */
        size_t getFontDataSize() const {
            return fontData.size();
        }

        uint32_t getTableOffset(const std::string& tag) const {
            uint32_t tagVal = *(uint32_t*)tag.data();
            for (const auto& entry : offsetTable.tableDirectoryEntries) {
                if (entry.tag == tagVal) {
                    return entry.offset;
                }
            }
            return 0;  // Return 0 if the table is not found
        }

        const std::vector<TableDirectoryEntry>& getTableDirectoryEntries() {
            return offsetTable.tableDirectoryEntries;
        }

        bool parseCmapFormat0(uint32_t offset, CmapFormat0& table);
        bool parseCmapFormat2(uint32_t offset, CmapFormat2& table);
        bool parseCmapFormat4(uint32_t offset, CmapFormat4& table);
        bool parseCmapFormat6(uint32_t offset, CmapFormat6& table);
        bool parseCmapFormat8(uint32_t offset, CmapFormat8& table);
        bool parseCmapFormat10(uint32_t offset, CmapFormat10& table);
        bool parseCmapFormat12(uint32_t offset, CmapFormat12& table);
        bool parseCmapFormat13(uint32_t offset, CmapFormat13& table);
        bool parseCmapFormat14(uint32_t offset, CmapFormat14& table);

        bool parseGPOSHeader(uint32_t offset, GPOSHeader& header);

        bool parseScriptList(uint32_t offset, std::vector<ScriptRecord>& scripts);
        bool parseFeatureList(uint32_t offset, std::vector<FeatureRecord>& features);
        bool parseLookupList(uint32_t offset, std::vector<LookupTable>& lookups);
        bool parseSinglePos(uint32_t offset, SinglePos& singlePos);
        bool parseScriptTable(uint32_t offset, ScriptTable& scriptTable);
        bool parseSingleAdjustmentSubtable(uint32_t offset, SingleAdjustmentSubtable& subtable);
        bool parseValueRecord(uint32_t& offset, uint16_t valueFormat, ValueRecord& value);
        uint16_t getValueRecordSize(uint16_t valueFormat);
        uint16_t getCoverageGlyphCount(uint32_t offset);

        uint32_t getTableLength(const std::string& tableName);

    private:
        // Read the offset table and directory entries
        bool readOffsetTable();

        // Compound Glyph Flags
        // These flags are used when parsing compound glyphs, which consist of two or more simple glyphs combined.
        // - ARG_1_AND_2_ARE_WORDS: Indicates that the arguments are words instead of bytes.
        // - MORE_COMPONENTS: Indicates that there are more components to follow.
        // - WE_HAVE_A_SCALE: Indicates that scaling is applied.
        // - WE_HAVE_AN_X_AND_Y_SCALE: Indicates that an X direction scale is applied separately to the horizontal and vertical components of the glyph.
        // - WE_HAVE_A_TWO_BY_TWO: Indicates that a 2x2 transformation is applied; i.e., scale and 90-degree rotation (used to implement oblique typefaces).
        // - WE_HAVE_INSTRUCTIONS: Indicates that there are instructions for the compound glyph.
        // - USE_MY_METRICS: Indicates that the composite is not to be scaled, rotated or translated.
        // - OVERLAP_COMPOUND: Indicates that the composite is designed to have the component glyphs overlap.
        // See https://docs.microsoft.com/en-us/typography/opentype/spec/glyf#compound-glyph-description for more information.
        static const uint16_t ARG_1_AND_2_ARE_WORDS = 0x0001;
        static const uint16_t MORE_COMPONENTS = 0x0020;
        static const uint16_t WE_HAVE_A_SCALE = 0x0008;
        static const uint16_t WE_HAVE_AN_X_AND_Y_SCALE = 0x0040;
        static const uint16_t WE_HAVE_A_TWO_BY_TWO = 0x0080;
        static const uint16_t WE_HAVE_INSTRUCTIONS = 0x0100;
        static const uint16_t USE_MY_METRICS = 0x0200;
        static const uint16_t OVERLAP_COMPOUND = 0x0400;

        // Private Data Members
        OffsetTable offsetTable; // Offset table of the TTF file.
        std::map<std::string, std::vector<uint8_t>> tableData; // Maps table tags to their data.

        std::vector<uint8_t> fontData; // Entire TTF font data loaded from file.
        uint16_t numGlyphs = 0; // Number of glyphs in the font.
    };

} // namespace TTFParser

#endif // TTF_PARSER_HPP