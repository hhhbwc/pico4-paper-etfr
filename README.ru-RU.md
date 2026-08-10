# PICO 4 + Paper Eye Tracker → системный рендеринг с отслеживанием взгляда (ETFR)

Проект по реверс-инжинирингу: заставить **PICO 4 (A8110)** использовать **системный рендеринг с отслеживанием взгляда (Eye-Tracked Foveated Rendering, ETFR)** с помощью стороннего трекера глаз **Paper Tracker**.

> 中文: [README.md](README.md) · English: [README.en-US.md](README.en-US.md)

## Цель

У **стандартной версии PICO 4 нет аппаратного отслеживания глаз**, но система (та же, что и в Pro-версии) **полностью содержит программную цепочку отслеживания глаз + рендеринга с фовеацией**. Проект внедряет данные стороннего трекера (Paper), чтобы система поверила, что у неё есть нативное отслеживание глаз, и включила ETFR (рендеринг следует за взглядом для экономии GPU).

## Ключевые выводы (подтверждено реверс-инжинирингом)

### 1. Paper передаёт «изображения», а не «координаты»
- Датчики — **3 USB-устройства ESP32-S3**: левый глаз (0425:0002) / правый глаз (0425:0003) / лицо (0425:0001)
- Приложение PICO (`com.bridge.papertracker`) читает их напрямую через **libusb** (без ядерного драйвера FTDI)
- **PICO → ПК отправляет только изображения глаз (240×320 ч/б JPEG)**, UDP-unicast `192.168.1.119:35506 → ПК:45454`
- **Координаты взгляда вычисляются на ПК из изображения** (в этом суть «ПК обрабатывает данные» Paper)
- Протокол кадра: заголовок `PT` (5054 0101) + счётчик кадра + индекс фрагмента + длина + JPEG

### 2. Слой рендеринга PICO имеет полный API ETFR (цепочка открыта)
Экспортируется из `libopenxr_api.so`:
- `Pxr_GetEyeTrackingData` / `Pxr_GetEyeTrackingData1`
- `Pxr_StartEyeTracking` / `Pxr_StopEyeTracking`
- `Pxr_GetEyeTrackingSupported` / `Pxr_GetEyeTrackingState`
- `Pxr_SetFoveationLevel` / `Pxr_SetFoveationParams` / `Pxr_GetLayerFoveationImage`

**Стандартная версия использует ту же систему, что и Pro → цепочка потребления существует, не хватает только источника данных.**

### 3. Сервис отслеживания глаз полон, но «простаивает»
`pxreyetrackingservice` (зарегистрированный binder `pvr.IEyeTrackingService`):
```
The status of eye camera: Nonexistent   <- в стандартной версии нет камеры глаз
ET algorithm is not started.
isSupportEyetracking = 0
```
Сервис полностью функционален и вызывается через binder, просто **не запускается, т.к. не обнаруживает оборудования. `isSupportEyetracking` — программная проверка (можно хукнуть)**.

### 4. Внутренняя архитектура
`libpxreyetrackingservice.so` / `libpxreyetracking.phoenix.so`:
```
CameraManager::openCamera/addImageListener → onFrameAvailable  <- изображение с камеры
AlgorithmBase::setResultsListener / getTrackingDataSharedMemory  <- алгоритм
TrackingService::GetData / SetData / GetTrackingDataSharedMemory  <- вывод данных
```

## Путь внедрения (определён, реализуется)

**План (перехватить систему, переиспользовать нативный алгоритм + рендеринг):**
1. **Хукнуть `isSupportEyetracking` → вернуть true** (главный переключатель)
2. **Заставить CameraManager думать, что камера есть**: подать изображения Paper в систему
3. Запустится алгоритм → вычислит взгляд → рендеринг потребит → **ETFR включён**

**Или альтернатива: хукнуть `Pxr_GetEyeTrackingData` / `SetData`, чтобы внедрять координаты напрямую.**

## Инструменты (`tools/`)
- `usbcdc_read.c` — читатель USB CDC (сборка NDK)
- `ptrace_probe.c` — пробник внедрения ptrace (подтверждена работоспособность attach)
- `list_9100.ps1` / `cap_osc.ps1` — анализ OSC/сети на стороне ПК

## TODO / Блокеры
- [ ] Внедрить ptrace в `pxreyetrackingservice` (root-системный сервис, нужна отдельная сессия)
- [ ] Хукнуть `isSupportEyetracking` + внедрить источник данных
- [ ] Реверс структуры данных алгоритма (формат взгляда DataBufferParcelable)
- [ ] Тест в VR: реагирует ли рендеринг (следует ли фовеация)

## Лицензия
MIT
