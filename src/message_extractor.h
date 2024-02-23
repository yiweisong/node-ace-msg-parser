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
    uint8_t* m_unparsedData;
    size_t m_unparsedDataSize;

    bool m_needCross;
    size_t m_crossStart;
    size_t m_crossSize;
  private:
    void _handle_extracted_data(Napi::Env &env, Napi::Array &list);
    void _clear_cross_status();
    void _clear_unparsed_data();
    void _reset_message_analyzers();
  };
}

#endif