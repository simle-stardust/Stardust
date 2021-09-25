

//#define FLIGHT_BOX
#define GROUND_BOX

#if (defined FLIGHT_BOX) && (defined GROUND_BOX)
  #error "Define only one config"
#elif !(defined FLIGHT_BOX) && !(defined GROUND_BOX)
  #error "Define one config"
#endif

