// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"
#include <SoapySDR/Version.hpp>
#ifdef SOAPY_SDR_API_HAS_ERR_TO_STR
#include <SoapySDR/Errors.hpp>
#endif //SOAPY_SDR_API_HAS_ERR_TO_STR
#include <algorithm> //min/max

class SDRSink : public SDRBlock
{
public:
    static Block *make(const Pothos::DType &dtype, const std::vector<size_t> &channels)
    {
        return new SDRSink(dtype, channels);
    }

    SDRSink(const Pothos::DType &dtype, const std::vector<size_t> &channels):
        SDRBlock(SOAPY_SDR_TX, dtype, channels)
    {
        for (size_t i = 0; i < _channels.size(); i++) this->setupInput(i, dtype);
    }

    /*******************************************************************
     * Streaming implementation
     ******************************************************************/
    void work(void)
    {
        int flags = 0;
        long long timeNs = 0;
        size_t numElems = this->workInfo().minInElements;
        if (numElems == 0) return;

        //parse labels (from input 0)
        for (const auto &label : this->input(0)->labels())
        {
            //skip out of range labels
            if (label.index >= numElems) break;

            //found a time label
            if (label.id == "txTime")
            {
                if (label.index == 0) //time for this packet
                {
                    flags |= SOAPY_SDR_HAS_TIME;
                    timeNs = label.data.convert<long long>();
                }
                else //time on the next packet
                {
                    //truncate to not include this time label
                    numElems = label.index;
                    break;
                }
            }
            //found an end label
            if (label.id == "txEnd")
            {
                flags |= SOAPY_SDR_END_BURST;
                numElems = std::min<size_t>(label.index+label.width, numElems);
                break;
            }
        }

        //write the stream data
        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
        const auto &buffs = this->workInfo().inputPointers;
        const int ret = _device->writeStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);

        //handle result
        if (ret > 0) for (auto input : this->inputs()) input->consume(size_t(ret));
        else if (ret == SOAPY_SDR_TIMEOUT) return this->yield();
        else
        {
            for (auto input : this->inputs()) input->consume(numElems); //consume error region
            #ifdef SOAPY_SDR_API_HAS_ERR_TO_STR
            throw Pothos::Exception("SDRSink::work()", "writeStream "+std::string(SoapySDR::errToStr(ret)));
            #else
            throw Pothos::Exception("SDRSink::work()", "writeStream "+std::to_string(ret));
            #endif
        }
    }
};

static Pothos::BlockRegistry registerSDRSink(
    "/sdr/sink", &SDRSink::make);
