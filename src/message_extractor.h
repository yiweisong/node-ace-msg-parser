#ifndef MESSAGE_EXTRACTOR_H
#define MESSAGE_EXTRACTOR_H

#include <napi.h>
#include <memory>
#include "analyzers/message_analyzer.h"

namespace Aceinna
{
  class MessageExtractor: public Napi::ObjectWrap<MessageExtractor> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    MessageExtractor(const Napi::CallbackInfo&);
    Napi::Value Receive(const Napi::CallbackInfo&);
    void Reset(const Napi::CallbackInfo&);

  private:
    static constexpr const char JSClassName[] = "MessageExtractor";
    int m_determinedIndex;
    AnalysisResult m_analysisResult;
    std::vector<std::shared_ptr<MessageAnalyzer>> m_messageAnalyzers;
    uint8_t m_actualData[65536];
    uint8_t m_unparsedData[65536];
    size_t m_unparsedDataSize = 0;

    bool m_needCross = false;
    size_t m_crossStart = 0;
    size_t m_crossSize = 0;
  private:
    void _handle_extracted_data(Napi::Env &env, Napi::Array &list, size_t index);
    void _clear_cross_status();
    void _clear_unparsed_data();
    void _reset_message_analyzers();
  };
}

#endif