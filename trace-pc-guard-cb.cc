// trace-pc-guard-cb.cc
#include <stdint.h>
#include <stdio.h>
#include <sanitizer/coverage_interface.h>

// guard is branch

// This callback is inserted by the compiler as a module constructor
// into every DSO. 'start' and 'stop' correspond to the
// beginning and end of the section with the guards for the entire
// binary (executable or DSO). The callback will be called at least
// once per DSO and may be called multiple times with the same parameters.

/**
 * @brief 이 콜백은 컴파일러에 의해 모든 DSO(공유 객체 파일)에 모듈 생성자로 삽입됩니다.
 * 'start'와 'stop'은 이진 파일(실행 파일 또는 DSO) 전체에 대한 가드를 위한 섹션의 시작과 끝을 의미합니다.
 * 이 콜백은 최소한 DSO 당 한 번 이상 호출되며 동일한 매개변수로 여러 번 호출될 수도 있습니다.
 */
extern "C" void __sanitizer_cov_trace_pc_guard_init(uint32_t *start/*guard 변수 시작 부분 __TMC_END__*/,
                                                    uint32_t *stop/*guard 변수 끝 부분 __bss_start*/) {
  static uint64_t N;  // Counter for the guards.(branch)
  if (start == stop || *start) return;  // Initialize only once.
  printf("guard section: %p ~ %p\n", start, stop);
  for (uint32_t *x = start; x < stop; x++)
    *x = ++N;  // Guards should start from 1.
}

// This callback is inserted by the compiler on every edge in the
// control flow (some optimizations apply).
// Typically, the compiler will emit the code like this:
//    if(*guard)
//      __sanitizer_cov_trace_pc_guard(guard);
// But for large functions it will emit a simple call:
//    __sanitizer_cov_trace_pc_guard(guard);


/**
 * @brief 이 콜백은 컴파일러에 의해 제어 흐름의 모든 엣지(일부 최적화가 적용됨)에 삽입됩니다.
 * 일반적으로, 컴파일러는 다음과 같은 코드를 생성합니다:
 * if (*guard) // branch가 참인경우 실행
 *    __sanitizer_cov_trace_pc_guard(guard);
 * 그러나 큰 함수(복잡한 함수)의 경우 다음과 같이 간단한 호출을 생성할 수도 있습니다:
 * (조건 체크를 하지 않음 즉, branch인 경우 바로 점프)
 *    __sanitizer_cov_trace_pc_guard(guard);
 */
extern "C" void __sanitizer_cov_trace_pc_guard(uint32_t *guard) {
  if (!*guard) return;  // Duplicate the guard check.
  // If you set *guard to 0 this code will not be called again for this edge.
  // Now you can get the PC and do whatever you want:
  //   store it somewhere or symbolize it and print right away.
  // The values of `*guard` are as you set them in
  // __sanitizer_cov_trace_pc_guard_init and so you can make them consecutive
  // and use them to dereference an array or a bit vector.


  /**
   * @brief 가드 확인을 복제합니다.
   * 만약 *guard를 0(not taken)으로 설정하면 해당 엣지에 대해 이 코드가 다시 호출되지 않습니다.
   * 이제 PC(Program Counter)를 얻을 수 있으며 원하는 대로 처리할 수 있습니다:
   * 어딘가에 저장하거나 심볼화하여 즉시 출력할 수 있습니다.
   * *guard의 값은 __sanitizer_cov_trace_pc_guard_init에서 설정한 대로입니다.
   * 그래서 이 값들을 연속적으로 만들고 배열이나 비트 벡터를 역참조하는 데 사용할 수 있습니다.
   */
  void *PC = __builtin_return_address(0);
  char PcDescr[1024];
  /**
   * @brief 이 함수는 샌드박스 실행 시스템의 일부입니다.
   * 사용하려면 AddressSanitizer 또는 다른 샌드박스 실행 시스템과 링크하십시오.
   */
  // This function is a part of the sanitizer run-time.
  // To use it, link with AddressSanitizer or other sanitizer.
  __sanitizer_symbolize_pc(PC, "%p %F %L", PcDescr, sizeof(PcDescr));
  printf("guard: %p %x PC %s\n", guard, *guard, PcDescr);
}
