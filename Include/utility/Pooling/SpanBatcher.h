#pragma once
#include <vector>
#include "SubUpdate.h"

namespace utility {
    class SpanBatcher {
    private:
        std::vector<SubUpdate> m_updates;
        size_t m_threshold;
    public:
        SpanBatcher() : m_threshold(0) {

        }
        SpanBatcher(size_t Threshold) : m_threshold(Threshold) {

        }

        void emplace(SubUpdate update) {
            if (m_updates.empty()) {
                m_updates.emplace_back(update);
            }
            else
            {
                bool inserted = false;

                size_t mergeEntry = 0;
                for (size_t i = 0; i < m_updates.size() && !inserted; i++) {

                    SubUpdate& current = m_updates[i];
                    if (current.Intersects(update)) {
                        current.Merge(update, m_threshold);
                        inserted = true;
                        mergeEntry = i;
                    }
                    else
                        if (current.End < update.From) {
                            m_updates.insert(m_updates.begin() + i + 1, update);
                            inserted = true;
                            mergeEntry = i + 1;
                        }
                }
                if (!inserted) {
                    m_updates.emplace_back(update);
                }
                else {
                    size_t mergeExit;
                    for (mergeExit = mergeEntry; mergeExit < m_updates.size(); mergeExit++) {
                        SubUpdate current = m_updates[mergeExit];
                        if (!current.Intersects(update)) {
                            break;
                        }
                    }
                    Merge(mergeEntry, mergeExit);
                }
            }
        }
        void Merge(size_t first, size_t end) {
            m_updates[first].Merge(m_updates[end - 1], m_threshold);
            m_updates.erase(m_updates.begin() + static_cast<int64_t>(first + 1ULL), m_updates.begin() + static_cast<int64_t>(end));
        }

        void reset() {
            m_updates.clear();
        }
        SubUpdate* begin() {
            return m_updates.data();
        }
        SubUpdate* end() {
            return m_updates.data() + m_updates.size();
        }
    };
}