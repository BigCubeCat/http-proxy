#pragma once

#include <string>

enum class parser_stage { BEGIN_STAGE, READ_STAGE, WRITE_STAGE, END_STAGE };

/*!
 * Интерфейс процессинга соединения
 */
class connection_processor_iface {
public:
    virtual bool process()       = 0;
    virtual std::string result() = 0;
    virtual parser_stage stage() = 0;

    virtual ~connection_processor_iface() = default;
};
