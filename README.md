# code_coverage_example
## How To Compile
```bash
$ clang++ -g  -fsanitize-coverage=trace-pc-guard trace-pc-guard-example.cc -c
$ clang++ trace-pc-guard-cb.cc trace-pc-guard-example.o -fsanitize=address
```
### Running a Program
```bash
$ ASAN_OPTIONS=strip_path_prefix=`pwd`/ ./a.out
guard section: 0x562910985c60 ~ 0x562910985c70
guard: 0x562910985c64 2 PC 0x56291094e8ea in main trace-pc-guard-example.cc:4
guard: 0x562910985c68 3 PC 0x56291094e919 in main trace-pc-guard-example.cc:5:7
```

## 분석
```bash
$ readelf -a ./a.out
Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
	[22] .init_array       INIT_ARRAY       0000000000112600  00111600
       0000000000000020  0000000000000008  WA       0     0     8

.init_array[1]
0x00000000 00 0d e9 40 # sancov.module_ctor_trace_pc_guard
```
### disassemble
```bash
$ objdump -M intel -d ./a.out
00000000000de940 <sancov.module_ctor_trace_pc_guard>:
   de940:	55                   	push   rbp
   de941:	48 89 e5             	mov    rbp,rsp
   de944:	48 8d 3d 15 73 03 00 	lea    rdi,[rip+0x37315]        # 115c60 <__TMC_END__>, guard를 위한 섹션 시작 주소
   de94b:	48 8d 35 1e 73 03 00 	lea    rsi,[rip+0x3731e]        # 115c70 <__bss_start>, guard를 위한 섹션 끝 주소
   de952:	e8 b9 fb ff ff       	call   de510 <__sanitizer_cov_trace_pc_guard_init>
   de957:	5d                   	pop    rbp
   de958:	c3                   	ret    
   de959:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
```
### C code
```C
// trace-pc-guard-cb.cc
extern "C" void __sanitizer_cov_trace_pc_guard_init(
    uint32_t *start/*guard 변수 시작 부분 __TMC_END__*/,
    uint32_t *stop/*guard 변수 끝 부분 __bss_start*/) {

  static uint64_t N;  // Counter for the guards.(branch)
  if (start == stop || *start) return;  // Initialize only once.
  printf("guard section: %p ~ %p\n", start, stop);
  for (uint32_t *x = start; x < stop; x++)
    *x = ++N;  // Guards should start from 1.

}
```
### disassemble
**모든 branch 앞**에서 ```__sanitizer_cov_trace_pc_guard``` 가 호출, 총 4번 호출 : uint32 * 4번 = 16(0x10)
```bash
00000000000de8b0 <_Z3foov>:
   de8b0:	55                   	push   rbp
   de8b1:	48 89 e5             	mov    rbp,rsp
   de8b4:	48 8d 3d a5 73 03 00 	lea    rdi,[rip+0x373a5]        # 115c60 <__TMC_END__>

   de8bb:	e8 60 fd ff ff       	call   de620 <__sanitizer_cov_trace_pc_guard>
   de8c0:	5d                   	pop    rbp
   de8c1:	c3                   	ret

   de8c2:	66 2e 0f 1f 84 00 00 	cs nop WORD PTR [rax+rax*1+0x0]
   de8c9:	00 00 00 
   de8cc:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]

00000000000de8d0 <main>:
   de8d0:	55                   	push   rbp
   de8d1:	48 89 e5             	mov    rbp,rsp
   de8d4:	48 83 ec 20          	sub    rsp,0x20
   de8d8:	89 7d e4             	mov    DWORD PTR [rbp-0x1c],edi
   de8db:	48 89 75 e8          	mov    QWORD PTR [rbp-0x18],rsi

   de8df:	48 8d 3d 7e 73 03 00 	lea    rdi,[rip+0x3737e]        # 115c64 <__TMC_END__+0x4>
   de8e6:	e8 35 fd ff ff       	call   de620 <__sanitizer_cov_trace_pc_guard>

   de8eb:	8b 7d e4             	mov    edi,DWORD PTR [rbp-0x1c]
   de8ee:	48 8b 75 e8          	mov    rsi,QWORD PTR [rbp-0x18]
   de8f2:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [rbp-0x4],0x0
   de8f9:	89 7d f8             	mov    DWORD PTR [rbp-0x8],edi
   de8fc:	48 89 75 f0          	mov    QWORD PTR [rbp-0x10],rsi
   de900:	83 7d f8 01          	cmp    DWORD PTR [rbp-0x8],0x1
   de904:	0f 8f 15 00 00 00    	jg     de91f <main+0x4f>
   de90a:	48 8d 3d 53 73 03 00 	lea    rdi,[rip+0x37353]        # 115c64 <__TMC_END__+0x4>
   de911:	48 83 c7 04          	add    rdi,0x4

   de915:	e8 06 fd ff ff       	call   de620 <__sanitizer_cov_trace_pc_guard>
   de91a:	e9 15 00 00 00       	jmp    de934 <main+0x64>

   de91f:	48 8d 3d 3e 73 03 00 	lea    rdi,[rip+0x3733e]        # 115c64 <__TMC_END__+0x4>
   de926:	48 83 c7 08          	add    rdi,0x8

   de92a:	e8 f1 fc ff ff       	call   de620 <__sanitizer_cov_trace_pc_guard>
   de92f:	e8 7c ff ff ff       	call   de8b0 <_Z3foov>

   de934:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
   de937:	48 83 c4 20          	add    rsp,0x20
   de93b:	5d                   	pop    rbp
   de93c:	c3                   	ret  
```
