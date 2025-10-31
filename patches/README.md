# ioLibrary_Driver 패치 파일

이 디렉토리에는 ESP32-S3 W5500 프로젝트에서 사용하는 ioLibrary_Driver 서브모듈의 수정사항이 패치 형태로 저장되어 있습니다.

## 패치 파일 목록

### 1. `ioLibrary_Driver_detailed_changes.patch`
- **설명**: ioLibrary_Driver 서브모듈의 상세한 변경사항
- **포함 내용**:
  - W5500 SPI 드라이버 수정
  - Transaction-based SPI 지원 추가
  - ESP32 호환성 개선
  - 메모리 관리 개선

### 2. `ioLibrary_Driver_esp32_modifications.patch`
- **설명**: 메인 프로젝트 기준 서브모듈 변경사항
- **포함 내용**: 서브모듈 상태 변경

## 패치 적용 방법

### 1. 새로운 클론에서 패치 적용
```bash
# 1. 메인 프로젝트 클론
git clone <your-repo-url>
cd ESP32_W5500_TEST

# 2. 서브모듈 초기화
git submodule update --init --recursive

# 3. 서브모듈 디렉토리로 이동
cd lib/ioLibrary_Driver

# 4. 패치 적용
git apply ../../patches/ioLibrary_Driver_detailed_changes.patch
```

### 2. 기존 프로젝트에서 패치 적용
```bash
# 서브모듈 디렉토리에서
cd lib/ioLibrary_Driver

# 변경사항 되돌리기 (필요시)
git checkout .

# 패치 적용
git apply ../../patches/ioLibrary_Driver_detailed_changes.patch
```


## 주요 변경사항
### W5500.c
- Transaction-based SPI 드라이버 지원 추가
- ESP32 SPI 함수와 연동

