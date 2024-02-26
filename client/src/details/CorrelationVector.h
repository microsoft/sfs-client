// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <string>

namespace microsoft
{
class correlation_vector;
}

namespace SFS::details
{
class ReportingHandler;

class CorrelationVector
{
  public:
    CorrelationVector(const ReportingHandler& handler);
    ~CorrelationVector();

    /**
     * @brief Set a new correlation vector
     */
    void SetCorrelationVector(const std::string& cv);

    /**
     * @brief Returns the current correlation vector and increments the internal state
     */
    std::string PopLatestString();

  private:
    void Increment();

    const ReportingHandler& m_handler;
    std::unique_ptr<microsoft::correlation_vector> m_cv;
    bool m_isFirstUse = true;
};
} // namespace SFS::details
