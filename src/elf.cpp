#include "elf.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ari {

namespace {

class Writer {
public:
    explicit Writer(std::vector<std::uint8_t>& data) : data_(data) {}

    void seek(std::size_t at) {
        at_ = at;
        if (data_.size() < at_) data_.resize(at_, 0);
    }

    std::size_t tell() const { return at_; }

    void byte(std::uint8_t value) {
        if (at_ == data_.size()) data_.push_back(value);
        else data_[at_] = value;
        ++at_;
    }

    void u16(std::uint16_t value) {
        for (int i = 0; i < 2; ++i) byte(static_cast<std::uint8_t>((value >> (i * 8)) & 0xff));
    }

    void u32(std::uint32_t value) {
        for (int i = 0; i < 4; ++i) byte(static_cast<std::uint8_t>((value >> (i * 8)) & 0xff));
    }

    void u64(std::uint64_t value) {
        for (int i = 0; i < 8; ++i) byte(static_cast<std::uint8_t>((value >> (i * 8)) & 0xff));
    }

private:
    std::vector<std::uint8_t>& data_;
    std::size_t at_ = 0;
};

static void align_file(std::vector<std::uint8_t>& file, std::size_t alignment) {
    while (file.size() % alignment != 0) file.push_back(0);
}

static std::uint32_t add_string(std::vector<std::uint8_t>& table, const std::string& text) {
    std::uint32_t offset = static_cast<std::uint32_t>(table.size());
    table.insert(table.end(), text.begin(), text.end());
    table.push_back(0);
    return offset;
}

static void write_symbol(Writer& writer,
                         std::uint32_t name,
                         std::uint8_t info,
                         std::uint16_t section,
                         std::uint64_t value,
                         std::uint64_t size) {
    writer.u32(name);
    writer.byte(info);
    writer.byte(0);
    writer.u16(section);
    writer.u64(value);
    writer.u64(size);
}

static void write_section_header(Writer& writer,
                                 std::uint32_t name,
                                 std::uint32_t type,
                                 std::uint64_t flags,
                                 std::uint64_t address,
                                 std::uint64_t offset,
                                 std::uint64_t size,
                                 std::uint32_t link,
                                 std::uint32_t info,
                                 std::uint64_t addralign,
                                 std::uint64_t entsize) {
    writer.u32(name);
    writer.u32(type);
    writer.u64(flags);
    writer.u64(address);
    writer.u64(offset);
    writer.u64(size);
    writer.u32(link);
    writer.u32(info);
    writer.u64(addralign);
    writer.u64(entsize);
}

class ElfWriter {
public:
    static std::vector<std::uint8_t> write_executable(const std::vector<std::uint8_t>& code,
                                                      const std::vector<ElfSymbol>& symbols) {
        constexpr std::uint64_t base = 0x400000;
        constexpr std::uint64_t code_offset = 0x1000;

        std::vector<std::uint8_t> file(static_cast<std::size_t>(code_offset), 0);
        file.insert(file.end(), code.begin(), code.end());
        std::uint64_t load_size = static_cast<std::uint64_t>(file.size());

        std::uint64_t symtab_offset = 0;
        std::uint64_t symtab_size = 0;
        std::uint64_t strtab_offset = 0;
        std::uint64_t strtab_size = 0;
        std::uint64_t shstrtab_offset = 0;
        std::uint64_t shstrtab_size = 0;
        std::uint64_t section_header_offset = 0;
        std::uint16_t section_count = 0;
        std::uint16_t section_string_index = 0;

        if (!symbols.empty()) {
            std::vector<std::uint8_t> strtab;
            strtab.push_back(0);
            std::vector<std::uint32_t> symbol_name_offsets;
            symbol_name_offsets.reserve(symbols.size());
            for (const auto& symbol : symbols) {
                symbol_name_offsets.push_back(add_string(strtab, symbol.name));
            }

            align_file(file, 8);
            symtab_offset = static_cast<std::uint64_t>(file.size());
            Writer writer(file);
            writer.seek(file.size());
            write_symbol(writer, 0, 0, 0, 0, 0);
            for (std::size_t i = 0; i < symbols.size(); ++i) {
                const ElfSymbol& symbol = symbols[i];
                write_symbol(writer,
                             symbol_name_offsets[i],
                             static_cast<std::uint8_t>((1 << 4) | 2),
                             1,
                             base + code_offset + symbol.offset,
                             symbol.size);
            }
            symtab_size = static_cast<std::uint64_t>(writer.tell()) - symtab_offset;

            strtab_offset = static_cast<std::uint64_t>(file.size());
            file.insert(file.end(), strtab.begin(), strtab.end());
            strtab_size = static_cast<std::uint64_t>(strtab.size());

            std::vector<std::uint8_t> shstrtab;
            shstrtab.push_back(0);
            std::uint32_t sh_text = add_string(shstrtab, ".text");
            std::uint32_t sh_symtab = add_string(shstrtab, ".symtab");
            std::uint32_t sh_strtab = add_string(shstrtab, ".strtab");
            std::uint32_t sh_shstrtab = add_string(shstrtab, ".shstrtab");

            shstrtab_offset = static_cast<std::uint64_t>(file.size());
            file.insert(file.end(), shstrtab.begin(), shstrtab.end());
            shstrtab_size = static_cast<std::uint64_t>(shstrtab.size());

            align_file(file, 8);
            section_header_offset = static_cast<std::uint64_t>(file.size());
            writer.seek(file.size());
            write_section_header(writer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            write_section_header(writer, sh_text, 1, 0x6, base + code_offset, code_offset, code.size(), 0, 0, 16, 0);
            write_section_header(writer, sh_symtab, 2, 0, 0, symtab_offset, symtab_size, 3, 1, 8, 24);
            write_section_header(writer, sh_strtab, 3, 0, 0, strtab_offset, strtab_size, 0, 0, 1, 0);
            write_section_header(writer, sh_shstrtab, 3, 0, 0, shstrtab_offset, shstrtab_size, 0, 0, 1, 0);
            section_count = 5;
            section_string_index = 4;
        }

        Writer writer(file);
        writer.seek(0);
        writer.byte(0x7f);
        writer.byte('E');
        writer.byte('L');
        writer.byte('F');
        writer.byte(2);
        writer.byte(1);
        writer.byte(1);
        writer.byte(0);
        for (int i = 0; i < 8; ++i) writer.byte(0);
        writer.u16(2);
        writer.u16(62);
        writer.u32(1);
        writer.u64(base + code_offset);
        writer.u64(64);
        writer.u64(section_header_offset);
        writer.u32(0);
        writer.u16(64);
        writer.u16(56);
        writer.u16(1);
        writer.u16(64);
        writer.u16(section_count);
        writer.u16(section_string_index);

        writer.seek(64);
        writer.u32(1);
        writer.u32(5);
        writer.u64(0);
        writer.u64(base);
        writer.u64(base);
        writer.u64(load_size);
        writer.u64(load_size);
        writer.u64(0x1000);

        return file;
    }

    static std::vector<std::uint8_t> write_relocatable_object(const std::vector<std::uint8_t>& code,
                                                              const std::vector<ElfSymbol>& symbols,
                                                              const std::vector<ElfRelocation>& relocations) {
        const bool has_relocations = !relocations.empty();
        std::vector<std::uint8_t> file(64, 0);
        align_file(file, 16);
        std::uint64_t text_offset = static_cast<std::uint64_t>(file.size());
        file.insert(file.end(), code.begin(), code.end());

        std::vector<std::uint8_t> strtab;
        strtab.push_back(0);
        std::vector<std::uint32_t> symbol_name_offsets;
        symbol_name_offsets.reserve(symbols.size());
        std::map<std::string, std::uint32_t> symbol_indices;
        std::uint32_t next_symbol_index = 1;
        for (const auto& symbol : symbols) {
            symbol_name_offsets.push_back(add_string(strtab, symbol.name));
            if (!symbol_indices.count(symbol.name)) {
                symbol_indices[symbol.name] = next_symbol_index;
            }
            ++next_symbol_index;
        }

        std::vector<std::string> undefined_symbols;
        std::vector<std::uint32_t> undefined_name_offsets;
        for (const auto& relocation : relocations) {
            if (symbol_indices.count(relocation.symbol)) continue;
            symbol_indices[relocation.symbol] = next_symbol_index++;
            undefined_symbols.push_back(relocation.symbol);
            undefined_name_offsets.push_back(add_string(strtab, relocation.symbol));
        }

        std::uint64_t rela_text_offset = 0;
        std::uint64_t rela_text_size = 0;
        if (has_relocations) {
            align_file(file, 8);
            rela_text_offset = static_cast<std::uint64_t>(file.size());
            Writer rela_writer(file);
            rela_writer.seek(file.size());
            for (const auto& relocation : relocations) {
                auto found = symbol_indices.find(relocation.symbol);
                std::uint64_t symbol_index = found == symbol_indices.end() ? 0 : found->second;
                rela_writer.u64(relocation.offset);
                rela_writer.u64((symbol_index << 32) | relocation.type);
                rela_writer.u64(static_cast<std::uint64_t>(relocation.addend));
            }
            rela_text_size = static_cast<std::uint64_t>(rela_writer.tell()) - rela_text_offset;
        }

        align_file(file, 8);
        std::uint64_t symtab_offset = static_cast<std::uint64_t>(file.size());
        Writer writer(file);
        writer.seek(file.size());
        write_symbol(writer, 0, 0, 0, 0, 0);
        for (std::size_t i = 0; i < symbols.size(); ++i) {
            const ElfSymbol& symbol = symbols[i];
            write_symbol(writer,
                         symbol_name_offsets[i],
                         static_cast<std::uint8_t>((1 << 4) | 2),
                         1,
                         symbol.offset,
                         symbol.size);
        }
        for (std::size_t i = 0; i < undefined_symbols.size(); ++i) {
            write_symbol(writer,
                         undefined_name_offsets[i],
                         static_cast<std::uint8_t>((1 << 4) | 2),
                         0,
                         0,
                         0);
        }
        std::uint64_t symtab_size = static_cast<std::uint64_t>(writer.tell()) - symtab_offset;

        std::uint64_t strtab_offset = static_cast<std::uint64_t>(file.size());
        file.insert(file.end(), strtab.begin(), strtab.end());
        std::uint64_t strtab_size = static_cast<std::uint64_t>(strtab.size());

        std::vector<std::uint8_t> shstrtab;
        shstrtab.push_back(0);
        std::uint32_t sh_text = add_string(shstrtab, ".text");
        std::uint32_t sh_rela_text = has_relocations ? add_string(shstrtab, ".rela.text") : 0;
        std::uint32_t sh_symtab = add_string(shstrtab, ".symtab");
        std::uint32_t sh_strtab = add_string(shstrtab, ".strtab");
        std::uint32_t sh_shstrtab = add_string(shstrtab, ".shstrtab");

        std::uint64_t shstrtab_offset = static_cast<std::uint64_t>(file.size());
        file.insert(file.end(), shstrtab.begin(), shstrtab.end());
        std::uint64_t shstrtab_size = static_cast<std::uint64_t>(shstrtab.size());

        align_file(file, 8);
        std::uint64_t section_header_offset = static_cast<std::uint64_t>(file.size());
        writer.seek(file.size());
        write_section_header(writer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        write_section_header(writer, sh_text, 1, 0x6, 0, text_offset, code.size(), 0, 0, 16, 0);
        if (has_relocations) {
            write_section_header(writer, sh_rela_text, 4, 0, 0, rela_text_offset, rela_text_size, 3, 1, 8, 24);
            write_section_header(writer, sh_symtab, 2, 0, 0, symtab_offset, symtab_size, 4, 1, 8, 24);
            write_section_header(writer, sh_strtab, 3, 0, 0, strtab_offset, strtab_size, 0, 0, 1, 0);
            write_section_header(writer, sh_shstrtab, 3, 0, 0, shstrtab_offset, shstrtab_size, 0, 0, 1, 0);
        } else {
            write_section_header(writer, sh_symtab, 2, 0, 0, symtab_offset, symtab_size, 3, 1, 8, 24);
            write_section_header(writer, sh_strtab, 3, 0, 0, strtab_offset, strtab_size, 0, 0, 1, 0);
            write_section_header(writer, sh_shstrtab, 3, 0, 0, shstrtab_offset, shstrtab_size, 0, 0, 1, 0);
        }

        writer.seek(0);
        writer.byte(0x7f);
        writer.byte('E');
        writer.byte('L');
        writer.byte('F');
        writer.byte(2);
        writer.byte(1);
        writer.byte(1);
        writer.byte(0);
        for (int i = 0; i < 8; ++i) writer.byte(0);
        writer.u16(1);
        writer.u16(62);
        writer.u32(1);
        writer.u64(0);
        writer.u64(0);
        writer.u64(section_header_offset);
        writer.u32(0);
        writer.u16(64);
        writer.u16(0);
        writer.u16(0);
        writer.u16(64);
        writer.u16(has_relocations ? 6 : 5);
        writer.u16(has_relocations ? 5 : 4);

        return file;
    }
};

} // namespace

std::vector<std::uint8_t> write_elf_executable(const std::vector<std::uint8_t>& code,
                                               const std::vector<ElfSymbol>& symbols) {
    return ElfWriter::write_executable(code, symbols);
}

std::vector<std::uint8_t> write_elf_executable(const std::vector<std::uint8_t>& code) {
    return ElfWriter::write_executable(code, {});
}

std::vector<std::uint8_t> write_elf_relocatable_object(const std::vector<std::uint8_t>& code,
                                                       const std::vector<ElfSymbol>& symbols) {
    return ElfWriter::write_relocatable_object(code, symbols, {});
}

std::vector<std::uint8_t> write_elf_relocatable_object(const std::vector<std::uint8_t>& code,
                                                       const std::vector<ElfSymbol>& symbols,
                                                       const std::vector<ElfRelocation>& relocations) {
    return ElfWriter::write_relocatable_object(code, symbols, relocations);
}

} // namespace ari
