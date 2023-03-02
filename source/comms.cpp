#include "comms.h"
#include "gnclink.h"

#include <deque>
#include <mutex>
#include <iostream>
#include <format>

namespace Comms {
	uint8_t rxpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	uint8_t txpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	uint8_t rxframe[GNCLINK_FRAME_TOTAL_LENGTH];
	uint8_t txframe[GNCLINK_FRAME_TOTAL_LENGTH];

	// returns false if resend is being requested
	bool GetPacket(Serial::Port& port) {
        bool receivedFrames[GNCLINK_MAX_FRAMES_PER_PACKET];
        for (int i = 0; i < GNCLINK_MAX_FRAMES_PER_PACKET; ++i) receivedFrames[i] = false;

        // construct packet from frames
        while (1) {
            // receive data
            //serial_read_start(PORT0, rxframe, GNCLINK_FRAME_TOTAL_LENGTH);
            //// wait until data arrives
            //serial_read_wait_until_complete(PORT0);
            Serial::Read(port, rxframe, GNCLINK_FRAME_TOTAL_LENGTH);

            // check frame
            if (!GNClink_Check_Frame(rxframe)) {
                // continue, frame will be requested again later
                continue;
            }

            // check if frame is requesting resend
            if (GNClink_Frame_RequestResend(rxframe)) {
                return false;
            }

            // indicate frame has been received
            receivedFrames[GNClink_Get_Frame_Index(rxframe)] = true;

            // extract contents and place in packet
            bool moreFrames = true;
            GNClink_Reconstruct_Packet_From_Frames(rxframe, rxpacket, &moreFrames);

            // if no more frames, check if any need to be resent
            if (!moreFrames) {
                GNClink_FramePayload_RequestResend* payload = (GNClink_FramePayload_RequestResend*)GNClink_Get_Frame_Payload_Pointer(txframe);
                int resendCount = 0;
                for (int i = 0; i < GNClink_Get_Frame_Index(rxframe); ++i) {
                    if (!receivedFrames[i]) payload->resendIndexes[resendCount++] = (uint8_t)i;
                }
                // if resend is required
                if (resendCount) {
                    payload->resendCount = (uint8_t)resendCount;

                    GNClink_Construct_RequestResendFrame(txframe);

                    // send frame
                    //serial_write_start(PORT0, txframe, GNCLINK_FRAME_TOTAL_LENGTH);
                    //serial_write_wait_until_complete(PORT0);
                    Serial::Write(port, txframe, GNCLINK_FRAME_TOTAL_LENGTH);
                }
                // packet fully received
                else break;
            }
        }
        return true;
	}

	bool SendPacket(Serial::Port& port, bool resendFrames) {
        int count = 0;
        bool moreFrames = true;
        while (moreFrames) {
            int frameIndex = count;
            uint8_t frameFlags = GNClink_FrameFlags_None;
            // check if frames are being resent
            if (resendFrames) {
                GNClink_FramePayload_RequestResend* payload = (GNClink_FramePayload_RequestResend*)GNClink_Get_Frame_Payload_Pointer(rxframe);
                // break if all have been sent
                if (count == payload->resendCount) break;
                if (count == payload->resendCount - 1) frameFlags |= GNClink_FrameFlags_TransactionEnd;
                frameIndex = payload->resendIndexes[count];
            }

            // send frames
            GNClink_Get_Frame(txpacket, txframe, (GNClink_FrameFlags)frameFlags, frameIndex, &moreFrames);

            // send frame
            /*serial_write_start(PORT0, txframe, GNCLINK_FRAME_TOTAL_LENGTH);
            serial_write_wait_until_complete(PORT0);*/
            //if (!resendFrames) txframe[1]++;
            Serial::Write(port, txframe, GNCLINK_FRAME_TOTAL_LENGTH);

            ++count;
        }
        return true;
	}

	bool CommsLoop(Serial::Port& port) {
        SendPacket(port, false);
        while (!GetPacket(port)) {
            SendPacket(port, true);
        }

		return true;
	}


	uint32_t GetGlobalHash(Serial::Port& port) {
		GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetGlobalHash, GNClink_PacketFlags_None, 0);

		CommsLoop(port);

		return *(uint32_t*)GNClink_Get_Packet_Payload_Pointer(rxpacket);
	}

    void test(Serial::Port& port) {
        uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(rxpacket);
        uint8_t* txpayload = GNClink_Get_Packet_Payload_Pointer(txpacket);
        uint16_t* idList = (uint16_t*)(txpayload + 1);

        *txpayload = 3;
        idList[0] = 2;
        idList[1] = 1;
        idList[2] = 5;

        GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetValueList, GNClink_PacketFlags_None, 7);

        CommsLoop(port);

        std::cout << std::format("{}\n", *(uint32_t*)rxpayload);
        std::cout << std::format("{}\n", *(float*)(rxpayload + 8));
    }
}