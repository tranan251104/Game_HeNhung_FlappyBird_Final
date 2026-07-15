================================================================================
  HƯỚNG DẪN: Thư viện libtouchgfx-float-abi-hard.a bị thiếu
================================================================================

LỖI: cannot find -l:libtouchgfx-float-abi-hard.a

NGUYÊN NHÂN:
- File libtouchgfx-float-abi-hard.a là thư viện precompiled của TouchGFX
- File này bị .gitignore (*.a) nên không được commit vào repository
- Thư viện phải được copy từ bản cài đặt TouchGFX

CÁCH KHẮC PHỤC:

Cách 1 - Từ STM32Cube Repository (khuyến nghị):
  1. Mở File Explorer, đi đến:
     C:\Users\<TênBạn>\STM32Cube\Repository\Packs\STMicroelectronics\X-CUBE-TOUCHGFX\
  
  2. Vào thư mục phiên bản mới nhất (vd: 4.26.0)
  
  3. Tìm file: Middlewares/ST/touchgfx/lib/core/cortex_m4f/gcc/libtouchgfx-float-abi-hard.a
  
  4. Copy file đó vào thư mục này (nơi đang đọc file README)

Cách 2 - Từ TouchGFX Designer:
  1. Nếu đã cài TouchGFX Designer, thư viện thường ở:
     C:\TouchGFX\<version>\lib\core\cortex_m4f\gcc\
  
  2. Copy libtouchgfx-float-abi-hard.a vào thư mục này

Cách 3 - Regenerate project:
  1. Mở STM32CubeMX, mở file .ioc của project
  2. Generate Code
  3. Mở TouchGFX Designer, mở file .touchgfx
  4. Generate Code
  5. Quay lại CubeMX và Generate Code lần nữa
  (Quá trình này sẽ copy thư viện vào đúng vị trí)

================================================================================
