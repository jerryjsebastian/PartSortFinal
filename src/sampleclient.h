/******************************************************************************
** Copyright (c) 2006-2021 Unified Automation GmbH. All rights reserved.
**
** Software License Agreement ("SLA") Version 2.7
**
** Unless explicitly acquired and licensed from Licensor under another
** license, the contents of this file are subject to the Software License
** Agreement ("SLA") Version 2.7, or subsequent versions
** as allowed by the SLA, and You may not copy or use this file in either
** source code or executable form, except in compliance with the terms and
** conditions of the SLA.
**
** All software distributed under the SLA is provided strictly on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
** AND LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
** LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
** PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the SLA for specific
** language governing rights and limitations under the SLA.
**
** The complete license agreement can be found here:
** http://unifiedautomation.com/License/SLA/2.7/
**
** Project: C++ OPC Client SDK sample code
**
******************************************************************************/
#ifndef SAMPLECLIENT_H
#define SAMPLECLIENT_H

#include "uabase.h"
#include "uaclientsdk.h"

using namespace UaClientSdk;

class SampleClient : public UaSessionCallback
{
    UA_DISABLE_COPY(SampleClient);
public:
    SampleClient();
    virtual ~SampleClient();

    // UaSessionCallback implementation ----------------------------------------------------
    virtual void connectionStatusChanged(OpcUa_UInt32 clientConnectionId, UaClient::ServerStatus serverStatus);
    // UaSessionCallback implementation ------------------------------------------------------

    // OPC UA service calls
    UaStatus connect();
    UaStatus disconnect();
    // UaStatus read();
    // UaStatus write();
    UaStatus readCam_req(UaString& loop);
    UaStatus readCam_nr(UaString& cam);
    UaStatus writeCam_rdy(OpcUa_Boolean ready);
    UaStatus writeCam_done(OpcUa_Boolean done);
    UaStatus writePos_XYZ(OpcUa_Int32 val, const UaString identifier);
    UaStatus writeRot(OpcUa_Boolean rot, const UaString identifier);
    UaStatus writeTray_empty(OpcUa_Boolean finish);

private:
    UaSession* m_pSession;
};

#endif // SAMPLECLIENT_H

