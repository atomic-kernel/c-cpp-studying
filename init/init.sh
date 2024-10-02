# -fdwarf2-cfi-asm -fplt :只是配置
# -floop-nest-optimize ：此选项已经存在多年，但仍未启用
# -fsection-anchors ： x86 不支持
# -mskip-rax-setup ：只支持x86_64 -mgeneral-regs-only
# -funroll-completely-grow-size ：目前没有发现作用
# -fstrict-volatile-bitfields ：很有意思
# -frecord-gcc-switches
# -finhibit-size-directive
# -ftree-vectorize ：本身没作用，显式打开相当于打开 -ftree-loop-vectorize -ftree-slp-vectorize，显示关闭相当于关闭 那俩；更多地使用浮点寄存器，是-O2到-O3负优化的罪魁祸首
# -fno-pcc-struct-return -freg-struct-return :二进制不兼容

cat > /bin/gcc-me << EOF
#!/bin/bash
gcc \\
    -fstack-reuse=all -fjump-tables -fbit-tests -fno-non-call-exceptions -fno-float-store -ftrivial-auto-var-init=uninitialized -fstdarg-opt -fno-conserve-stack -fzero-call-used-regs=skip -ffp-contract=fast \\
    -fomit-frame-pointer -fstrict-aliasing -fdelete-null-pointer-checks -foptimize-sibling-calls -ftree-loop-vectorize -ftree-slp-vectorize -ffast-math -fno-rounding-math -fexcess-precision=fast -fno-signed-zeros -fno-trapping-math -fallow-store-data-races \\
    -fdelete-dead-exceptions -ffinite-loops -fipa-pta -fdevirtualize-at-ltrans -fmerge-all-constants -funroll-loops -funroll-all-loops \\
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables \\
    -fstack-check=no -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all -fno-instrument-functions \\
    -std=gnu2x -D_GNU_SOURCE -g0 -Ofast -Wall -Wextra -Wstrict-prototypes -Wshadow "\$@"
EOF

cat > /bin/g++-me << EOF
#!/bin/bash
g++ \\
    -fstack-reuse=all -fjump-tables -fbit-tests -fno-non-call-exceptions -fno-float-store -ftrivial-auto-var-init=uninitialized -fstdarg-opt -fno-conserve-stack -fzero-call-used-regs=skip -ffp-contract=fast \\
    -fomit-frame-pointer -fstrict-aliasing -fdelete-null-pointer-checks -ffinite-loops -foptimize-sibling-calls -ftree-loop-vectorize -ftree-slp-vectorize -ffast-math -fno-rounding-math -fexcess-precision=fast -fno-signed-zeros -fno-trapping-math -fallow-store-data-races \\
    -fdelete-dead-exceptions -fipa-pta -fdevirtualize-at-ltrans -fmerge-all-constants -funroll-loops -funroll-all-loops \\
    -fstack-check=no -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all -fno-instrument-functions -fvtable-verify=none -fno-threadsafe-statics \\
    -std=gnu++23 -D_GNU_SOURCE -g0 -Ofast -Wall -Wextra -Wshadow "\$@"
EOF

chmod +x /bin/gcc-me /bin/g++-me

# clang 不支持的选项：
# -fstack-reuse=all -fbit-tests -fno-float-store -fstdarg-opt -fno-conserve-stack
# -fexcess-precision=fast -fallow-store-data-races
# -fdelete-dead-exceptions -fipa-pta -fdevirtualize-at-ltrans -funroll-all-loops
# -fno-instrument-functions
# 和GCC不同的选项：
# -ffinite-loops
# -fno-rounding-math -ffp-contract=fast
# -fslp-vectorize == -ftree-slp-vectorize
# -fvectorize == -ftree-loop-vectorize

cat > /usr/bin/clang-me << EOF
#!/bin/bash
clang \\
    -Xclang -pic-level -Xclang 0 -fno-addrsig \\
    -fno-float-store -fexcess-precision=fast -funroll-all-loops -Wno-ignored-optimization-argument -Wno-unused-command-line-argument \\
    -fjump-tables -fno-non-call-exceptions -ftrivial-auto-var-init=uninitialized -fzero-call-used-regs=skip -fno-rounding-math \\
    -fomit-frame-pointer -fstrict-aliasing -fdelete-null-pointer-checks -ffinite-loops -foptimize-sibling-calls -fvectorize -fslp-vectorize -ffast-math -ffp-contract=fast -fno-signed-zeros -fno-trapping-math -funroll-loops \\
    -fmerge-all-constants \\
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables \\
    -fno-stack-check -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all \\
    -std=gnu2x -D_GNU_SOURCE -g0 -Ofast -Wall -Wextra -Wstrict-prototypes -Wshadow "\$@"
EOF

cat > /usr/bin/clang++-me << EOF
#!/bin/bash
clang++ \\
    -Xclang -pic-level -Xclang 0 -fno-addrsig \\
    -fno-float-store -fexcess-precision=fast -funroll-all-loops -Wno-ignored-optimization-argument -Wno-unused-command-line-argument \\
    -fjump-tables -fno-non-call-exceptions -ftrivial-auto-var-init=uninitialized -fzero-call-used-regs=skip -fno-rounding-math \\
    -fomit-frame-pointer -fstrict-aliasing -fdelete-null-pointer-checks -ffinite-loops -foptimize-sibling-calls -fvectorize -fslp-vectorize -ffast-math -ffp-contract=fast -fno-signed-zeros -fno-trapping-math -funroll-loops \\
    -fmerge-all-constants \\
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables \\
    -fno-stack-check -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all \\
    -std=gnu++2b -D_GNU_SOURCE -g0 -Ofast -Wall -Wextra -Wstrict-prototypes -Wshadow "\$@"
EOF

chmod +x /bin/clang-me /bin/clang++-me

cat >> /usr/share/vim/vim90/defaults.vim <<EOF

set mouse-=a
set expandtab
set tabstop=4
set shiftwidth=4
set softtabstop=4
set autoindent
set smartindent
set cindent
set cinoptions+=g0
set hlsearch
EOF

cat >> ~/.inputrc << EOF
"\e[5~": history-search-backward
"\e[6~": history-search-forward
"\e[H": beginning-of-line
"\e[F": end-of-line
EOF

cat >> ~/.bashrc << EOF
HISTSIZE=-1
HISTFILESIZE=-1
HISTTIMEFORMAT='%F %T '
EOF

export LANG=zh_CN.utf8 LANGUAGE=zh_CN.utf8
echo "export LANG=zh_CN.utf8 LANGUAGE=zh_CN.utf8" >> "$HOME/.profile"
