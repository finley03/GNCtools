#include "comms.h"
#include "gnclink.h"

//#include <deque>
//#include <mutex>
#include <iostream>
#include <format>


namespace Comms {
	uint8_t rxpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	uint8_t txpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	uint8_t rxframe[GNCLINK_FRAME_TOTAL_LENGTH];
	uint8_t txframe[GNCLINK_FRAME_TOTAL_LENGTH];

    std::mutex mutex;

	// returns false if frame resend is being requested
	bool GetPacket(Serial::Port& port, bool& data_received) {
        data_received = true;

        bool receivedFrames[GNCLINK_MAX_FRAMES_PER_PACKET];
        for (int i = 0; i < GNCLINK_MAX_FRAMES_PER_PACKET; ++i) receivedFrames[i] = false;

        // construct packet from frames
        while (1) {
            
            // Check if data is received
            if (!Serial::Read(port, rxframe, GNCLINK_FRAME_TOTAL_LENGTH)) {
                // indicate data has not been received
                data_received = false;
                std::cout << "Connection timed out\n";
                return true;
            }

            // check frame
            if (!GNClink_Check_Frame(rxframe)) {
                std::cout << "WARNING: Frame failed check\n";
                // continue, frame will be requested again later
                continue;
            }

            // check if frame is requesting resend
            if (GNClink_Frame_RequestResend(rxframe)) {
                return false;
            }

            // indicate frame has been received
            receivedFrames[GNClink_Get_Frame_Index(rxframe)] = true;

            //std::cout << "Frame received\n";

            // extract contents and place in packet
            bool moreFrames = true;
            GNClink_Reconstruct_Packet_From_Frames(rxframe, rxpacket, &moreFrames);

            // if no more frames, check if any need to be resent
            if (!moreFrames) {
                GNClink_FramePayload_RequestResend* payload = (GNClink_FramePayload_RequestResend*)GNClink_Get_Frame_Payload_Pointer(txframe);
                int resendCount = 0;
                for (int i = 0; i < GNClink_Get_Frame_Index(rxframe); ++i) {
                    if (!receivedFrames[i]) {
                        payload->resendIndexes[resendCount++] = (uint8_t)i;
                        std::cout << std::format("Frame {} resend requested by host\n", i);
                    }
                }
                // if resend is required
                if (resendCount) {
                    payload->resendCount = (uint8_t)resendCount;

                    GNClink_Construct_RequestResendFrame(txframe);

                    // send frame
                    //serial_write_start(PORT0, txframe, GNCLINK_FRAME_TOTAL_LENGTH);
                    //serial_write_wait_until_complete(PORT0);
#ifdef COMMS_TEST
                    int random_value = std::rand() % 10; // random number between 0 and 9
                    if (random_value != 0) { // frame will not be sent with 1 in 10 chance
#endif
                    if (!Serial::Write(port, txframe, GNCLINK_FRAME_TOTAL_LENGTH)) return false; // error condition
#ifdef COMMS_TEST
                    }
                    else std::cout << "Frame resend request not sent by host\n";
#endif
                }
                // packet fully received
                else break;
            }
        }
        return true;
	}

	bool SendPacket(Serial::Port& port, bool resendFrames) {
        //std::cout << "\nNew packet\n";
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
                std::cout << std::format("Frame {} resend requested by device\n", frameIndex);
            }

            // send frames
            GNClink_Get_Frame(txpacket, txframe, (GNClink_FrameFlags)frameFlags, frameIndex, &moreFrames);

            // send frame
            /*serial_write_start(PORT0, txframe, GNCLINK_FRAME_TOTAL_LENGTH);
            serial_write_wait_until_complete(PORT0);*/
            //if (!resendFrames) txframe[1]++;

#ifdef COMMS_TEST
            int random_value = std::rand() % 10; // random number between 0 and 9
            if (random_value != 0) { // frame will not be sent with 1 in 10 chance
#endif
                if (!Serial::Write(port, txframe, GNCLINK_FRAME_TOTAL_LENGTH)) return false;
#ifdef COMMS_TEST
            }
            else std::cout << std::format("Frame {} not sent by host\n", frameIndex);
#endif

            ++count;
        }
        return true;
	}

	bool CommsLoop(Serial::Port& port) {
        bool data_received;
        bool packet_lost;

        do {
            // set previous received packet (and frame) to zero to avoid any propagation
            memset(rxpacket, 0, sizeof(rxpacket));
            memset(rxframe, 0, sizeof(rxframe));

            if (!SendPacket(port, false)) return false;
            while (!GetPacket(port, data_received)) { // if frame resend is requested send the packet again
                if (!SendPacket(port, true)) return false;
            }

            
            // check packet passes. If not redo transaction
            // If no packet was received, its contents will be zero and the packet will fail.
            packet_lost = !data_received || !GNClink_Check_Packet(rxpacket);
            if (packet_lost) {
                std::cout << "Packet lost\n";
            }
        } while (packet_lost);


		return true;
	}


	uint32_t GetGlobalHash(Serial::Port& port) {
		GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetGlobalHash, GNClink_PacketFlags_None, 0);

		CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_GetGlobalHash || (packetFlags & GNClink_PacketFlags_Response) == 0) return 0;

		return *(uint32_t*)GNClink_Get_Packet_Payload_Pointer(rxpacket);
	}

    uint16_t GetValueCount(Serial::Port& port) {
        GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetValueCount, GNClink_PacketFlags_None, 0);

        CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_GetValueCount || (packetFlags & GNClink_PacketFlags_Response) == 0) return 0;

        return *(uint16_t*)GNClink_Get_Packet_Payload_Pointer(rxpacket);
    }

    std::string GetValueName(uint16_t id, uint8_t& type, Serial::Port& port) {
        uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(rxpacket);
        uint16_t* txpayload = (uint16_t*)GNClink_Get_Packet_Payload_Pointer(txpacket);

        *txpayload = id;

        GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetValueName, GNClink_PacketFlags_None, 2);

        CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_GetValueName || (packetFlags & GNClink_PacketFlags_Response) == 0) return {};

        type = *rxpayload;

        return std::string((char*)(rxpayload + 1));
    }

    uint16_t GetProcessCount(Serial::Port& port) {
        GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetProcessCount, GNClink_PacketFlags_None, 0);

        CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_GetProcessCount || (packetFlags & GNClink_PacketFlags_Response) == 0) return 0;

        return *(uint16_t*)GNClink_Get_Packet_Payload_Pointer(rxpacket);
    }

    std::string GetProcessName(uint32_t id, Serial::Port& port) {
        uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(rxpacket);
        uint32_t* txpayload = (uint32_t*)GNClink_Get_Packet_Payload_Pointer(txpacket);

        *txpayload = id;

        GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetProcessName, GNClink_PacketFlags_None, 4);

        CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_GetProcessName || (packetFlags & GNClink_PacketFlags_Response) == 0) return {};

        return std::string((char*)(rxpayload));
    }

    // Returns false if no more processes to enumerate
    bool EnumerateProcesses(uint32_t& id, bool firstProcess, Serial::Port& port) {
        uint32_t* rxpayload = (uint32_t*)GNClink_Get_Packet_Payload_Pointer(rxpacket);
        uint32_t* txpayload = (uint32_t*)GNClink_Get_Packet_Payload_Pointer(txpacket);

        if (firstProcess) {
            GNClink_Construct_Packet(txpacket, GNClink_PacketType_EnumerateProcesses, GNClink_PacketFlags_None, 0);
        }
        else {
            *txpayload = id;
            GNClink_Construct_Packet(txpacket, GNClink_PacketType_EnumerateProcesses, GNClink_PacketFlags_None, 4);
        }

        CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_EnumerateProcesses || (packetFlags & GNClink_PacketFlags_Response) == 0) return true;

        int payloadLength = GNClink_Get_Packet_Payload_Size(rxpacket);
        if (payloadLength == 0) return false;
        else if (payloadLength == 4) {
            id = *rxpayload;
            return true;
        }
        else return true;
    }

    GNClink_PacketPayload_GetProcessDiagnostics_Response GetProcessDiagnostics(uint32_t id, Serial::Port& port) {
        GNClink_PacketPayload_GetProcessDiagnostics_Response* rxpayload =
            (GNClink_PacketPayload_GetProcessDiagnostics_Response*)GNClink_Get_Packet_Payload_Pointer(rxpacket);
        uint32_t* txpayload = (uint32_t*)GNClink_Get_Packet_Payload_Pointer(txpacket);

        *txpayload = id;
        GNClink_Construct_Packet(txpacket, GNClink_PacketType_GetProcessDiagnostics, GNClink_PacketFlags_None, 4);

        CommsLoop(port);

        if (!GNClink_Check_Packet(rxpacket)) std::cout << "WARNING: Packet failed check\n";

        GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
        GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
        if (packetType != GNClink_PacketType_GetProcessDiagnostics || (packetFlags & GNClink_PacketFlags_Response) == 0) return {};

        return *rxpayload;
    }
}