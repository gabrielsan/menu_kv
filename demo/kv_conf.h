#ifndef KV_CONF_H_
#define KV_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define KV_META_ATTR aligned (4)

#define KV_I32_LIST(OP)                                              \
    OP(PARAM1, 1, -10, 10, 0)                                        \
    OP(PARAM2, 2, -10, 10, 0)                                        \
    OP(PARAM3, -1, -10, 10, 0)                                       \
    OP(PARAM4, 4, -10, 10, 0)                                        \
    OP(PARAM5, 5, -10, 10, 0)                                        \
    OP(PARAM7, 0, -10, 10, 0)                                        \


#define KV_ENUM_LIST(OP)                                             \
    OP(P_ENUM_0, 0, 4, "hi", "how", "are", "you")                    \
    OP(P_ENUM_1, 0, 10)                                              \
    OP(P_ENUM_2, 0, 1008, "I", "am", "fine", "thank", "you")         \
    OP(P_ENUM_3, 1, 10, "")                                          \

#ifdef __cplusplus
}
#endif

#endif /* KV_CONF_H_ */
