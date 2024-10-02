#include <elf.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum elf_type
{
    ELF32 = 1,
    ELF64 = 2,
};

struct ELF_Header
{
    enum elf_type type;
    union
    {
        Elf32_Ehdr ehdr32;
        Elf64_Ehdr ehdr64;
    };
};

int parse_elf(struct ELF_Header *const header, FILE *const fp)
{
    rewind(fp);
    uint8_t head5[5];
    if (fread(&head5, 5, 1, fp) != 1)
        return -1;
    const uint8_t magic_model[4] = {0x7f, 'E', 'L', 'F'};
    if (memcmp(head5, magic_model, 4) != 0)
        return -1;

    if (head5[4] == 1) {
        rewind(fp);
        if (fread(&header->ehdr32, sizeof(header->ehdr32), 1, fp) != 1)
            return -1;
        header->type = ELF32;
    }
    else if (head5[4] == 2) {
        rewind(fp);
        if (fread(&header->ehdr64, sizeof(header->ehdr64), 1, fp) != 1)
            return -1;
        header->type = ELF64;
    }
    else
        return -1;

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fputs("Error: argc != 2 !\n", stderr);
        return -1;
    }
    FILE *const fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: fopen(%s) failed!\n", argv[1]);
        return -1;
    }
    struct ELF_Header elf_header;
    int ret = parse_elf(&elf_header, fp);
    if (ret) {
        fprintf(stderr, "Failed to parse elf!\n");
        goto label_error1;
    }
    void *buf0;
    void *buf1;

    if (elf_header.type == ELF64) {
        const Elf64_Ehdr *const p_ehdr = &elf_header.ehdr64;

        if (p_ehdr->e_shoff == 0) {
            ret = -1;
            fputs("没有节头表!\n", stderr);
            goto label_error1;
        }

        printf("节的个数：%hu\n", p_ehdr->e_shnum);
        printf("节字符串节的节号：%hu\n", p_ehdr->e_shstrndx);

        // Elf64_Shdr shdrs[p_ehdr->e_shnum]
        Elf64_Shdr *const shdrs = malloc(sizeof(Elf64_Shdr [p_ehdr->e_shnum]));
        if (shdrs == NULL) {
            ret = -1;
            fputs("error!\n", stderr);
            goto label_error1;
        }

        buf0 = shdrs;

        ret = fseek(fp, p_ehdr->e_shoff, SEEK_SET);
        if (ret) {
            fputs("error!\n", stderr);
            goto label_error2;
        }
        if (fread(shdrs, sizeof(Elf64_Shdr [p_ehdr->e_shnum]), 1, fp) != 1) {
            ret = -1;
            fputs("error!\n", stderr);
            goto label_error2;
        }
        ret = fseek(fp, shdrs[p_ehdr->e_shstrndx].sh_offset, SEEK_SET);
        if (ret) {
            fputs("error!\n", stderr);
            goto label_error2;
        }
        char *const shstr = malloc(shdrs[p_ehdr->e_shstrndx].sh_size);
        if (shstr == NULL) {
            ret = -1;
            fputs("error!\n", stderr);
            goto label_error2;
        }
        buf1 = shstr;
        if (fread(shstr, shdrs[p_ehdr->e_shstrndx].sh_size, 1, fp) != 1) {
            ret = -1;
            fputs("error!\n", stderr);
            goto label_error3;
        }

        for (size_t i = 0; i < p_ehdr->e_shnum; ++i) {
            printf("offset: %u\n", shdrs[i].sh_name);
            printf("%s\n", &shstr[shdrs[i].sh_name]);
        }

        free(shstr);
        free(shdrs);
    }


    ret = fclose(fp);
    if (ret)
        fprintf(stderr, "Error: fclose(%s)\n", argv[1]);
    return ret;

label_error3:
    free(buf1);
label_error2:
    free(buf0);
label_error1:
    fclose(fp);
    return ret;
}
