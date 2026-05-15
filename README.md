# test
application download

## LwM2M Object 9 C example

This repository now includes a focused C example that demonstrates the
Object 9 (Software Management) install flow:

- write **Software Package URI** (`/9/0/3`)
- transition **Update State** (`/9/0/7`) from `INITIAL` -> `DOWNLOAD_STARTED` -> `DOWNLOADED`
- execute **Install** (`/9/0/4`)
- complete install with **Update State** `INSTALLED` and **Update Result** (`/9/0/9`) `INSTALLED`

Example source:

- `/home/runner/work/test/test/examples/lwm2m_object9_install_demo.c`

Build and run:

```bash
cd /home/runner/work/test/test
cc -std=c11 -Wall -Wextra -pedantic -o lwm2m_object9_demo examples/lwm2m_object9_install_demo.c
./lwm2m_object9_demo
```

The example is intentionally self-contained and uses mocked download/install
logic so the Object 9 install state transitions are easy to inspect.
