## ioLibrary_Driver 변경사항 패치 파일 (상세 버전)

### 파일: ioLibrary_Driver_detailed_changes.patch

이 패치 파일은 ESP32-S3에서 WizNet W5500/W6300 Ethernet 컨트롤러를 사용하기 위해 
WizNet ioLibrary_Driver에 적용한 모든 변경사항을 포함합니다.
W5500과  W6300을의 포팅 작업을 위해 io라이브러리의 불가피한 수정사항이 발생
추후 ESP32에서  ioLibrary를 사용할 수 있도록 추가적인 지원을 계획중.


### 적용 방법:

```bash
# ioLibrary_Driver 디렉토리에서 실행
git apply ../../patches/ioLibrary_Driver_detailed_changes.patch
```

### 호환성:
- ESP-IDF v5.5.1+
- ESP32-S3 
- WizNet W5500/W6300
- Transaction-based SPI 우선 사용 권장

### 작성일: 2024-11-19
### 작성자: ESP32-S3 WizNet 통합 프로젝트