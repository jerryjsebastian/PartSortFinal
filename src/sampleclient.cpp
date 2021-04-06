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
#include "sampleclient.h"
#include "uasession.h"

SampleClient::SampleClient()
{
    m_pSession = new UaSession();
}

SampleClient::~SampleClient()
{
    if (m_pSession)
    {
        // disconnect if we're still connected
        if (m_pSession->isConnected() != OpcUa_False)
        {
            ServiceSettings serviceSettings;
            m_pSession->disconnect(serviceSettings, OpcUa_True);
        }
        delete m_pSession;
        m_pSession = NULL;
    }
}

void SampleClient::connectionStatusChanged(
    OpcUa_UInt32             clientConnectionId,
    UaClient::ServerStatus   serverStatus)
{
    OpcUa_ReferenceParameter(clientConnectionId);

    printf("-------------------------------------------------------------\n");
    switch (serverStatus)
    {
    case UaClient::Disconnected:
        printf("Connection status changed to Disconnected\n");
        break;
    case UaClient::Connected:
        printf("Connection status changed to Connected\n");
        break;
    case UaClient::ConnectionWarningWatchdogTimeout:
        printf("Connection status changed to ConnectionWarningWatchdogTimeout\n");
        break;
    case UaClient::ConnectionErrorApiReconnect:
        printf("Connection status changed to ConnectionErrorApiReconnect\n");
        break;
    case UaClient::ServerShutdown:
        printf("Connection status changed to ServerShutdown\n");
        break;
    case UaClient::NewSessionCreated:
        printf("Connection status changed to NewSessionCreated\n");
        break;
    }
    printf("-------------------------------------------------------------\n");
}

UaStatus SampleClient::connect()
{
    UaStatus result;

    // For now we use a hardcoded URL to connect to the local DemoServer
    // UaString sURL("opc.tcp://localhost:48010");
    UaString sURL("opc.tcp://r202113:53530/OPCUA/SimulationServer");

    // Provide information about the client
    SessionConnectInfo sessionConnectInfo;
    UaString sNodeName("unknown_host");
    char szHostName[256];
    if (0 == UA_GetHostname(szHostName, 256))
    {
        sNodeName = szHostName;
    }
    sessionConnectInfo.sApplicationName = "Unified Automation Getting Started Client";
    // Use the host name to generate a unique application URI
    sessionConnectInfo.sApplicationUri  = UaString("urn:%1:UnifiedAutomation:GettingStartedClient").arg(sNodeName);
    sessionConnectInfo.sProductUri      = "urn:UnifiedAutomation:GettingStartedClient";
    sessionConnectInfo.sSessionName     = sessionConnectInfo.sApplicationUri;

    // Security settings are not initialized - we connect without security for now
    SessionSecurityInfo sessionSecurityInfo;

    printf("\nConnecting to %s\n", sURL.toUtf8());
    result = m_pSession->connect(
        sURL,
        sessionConnectInfo,
        sessionSecurityInfo,
        this);

    if (result.isGood())
    {
        printf("Connect succeeded\n");
    }
    else
    {
        printf("Connect failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

UaStatus SampleClient::disconnect()
{
    UaStatus result;

    // Default settings like timeout
    ServiceSettings serviceSettings;

    printf("\nDisconnecting ...\n");
    result = m_pSession->disconnect(
        serviceSettings,
        OpcUa_True);

    if (result.isGood())
    {
        printf("Disconnect succeeded\n");
    }
    else
    {
        printf("Disconnect failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

UaStatus SampleClient::read()
{
    UaStatus          result;
    ServiceSettings   serviceSettings;
    UaReadValueIds    nodeToRead;
    UaDataValues      values;
    UaDiagnosticInfos diagnosticInfos;

    // Configure one node to read
    // We read the value of the ServerStatus -> CurrentTime
    nodeToRead.create(1);
    /*nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.Identifier.Numeric = OpcUaId_Server_ServerStatus_CurrentTime;*/

    nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.NamespaceIndex = 3;
    nodeToRead[0].NodeId.Identifier.Numeric = 1007;

    printf("\nReading ...\n");
    result = m_pSession->read(
        serviceSettings,
        0,
        OpcUa_TimestampsToReturn_Both,
        nodeToRead,
        values,
        diagnosticInfos);

    if (result.isGood())
    {
        // Read service succeded - check status of read value
        if (OpcUa_IsGood(values[0].StatusCode))
        {
            printf("ServerStatusCurrentTime: %s\n", UaVariant(values[0].Value).toString().toUtf8());
        }
        else
        {
            printf("Read failed for item[0] with status %s\n", UaStatus(values[0].StatusCode).toString().toUtf8());
        }
    }
    else
    {
        // Service call failed
        printf("Read failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

UaStatus SampleClient::readLoop(OpcUa_Boolean Loop)
{
    UaStatus          result;
    ServiceSettings   serviceSettings;
    UaReadValueIds    nodeToRead;
    UaDataValues      values;
    UaDiagnosticInfos diagnosticInfos;

    // Configure one node to read
    // We read the value of the ServerStatus -> CurrentTime
    nodeToRead.create(1);
    /*nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.Identifier.Numeric = OpcUaId_Server_ServerStatus_CurrentTime;*/

    nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.NamespaceIndex = 3;
    nodeToRead[0].NodeId.Identifier.Numeric = 1007;

    printf("\nReading ...\n");
    result = m_pSession->read(
        serviceSettings,
        0,
        OpcUa_TimestampsToReturn_Both,
        nodeToRead,
        values,
        diagnosticInfos);

    if (result.isGood())
    {
        // Read service succeded - check status of read value
        if (OpcUa_IsGood(values[0].StatusCode))
        {
            UaVariant(values[0].Value).toBool(Loop);
            printf("ServerStatusCurrentTime: %s\n", UaVariant(values[0].Value).toString().toUtf8());
        }
        else
        {
            printf("Read failed for item[0] with status %s\n", UaStatus(values[0].StatusCode).toString().toUtf8());
        }
    }
    else
    {
        // Service call failed
        printf("Read failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

UaStatus SampleClient::readLoop(UaString Loop)
{
    UaStatus          result;
    ServiceSettings   serviceSettings;
    UaReadValueIds    nodeToRead;
    UaDataValues      values;
    UaDiagnosticInfos diagnosticInfos;

    // Configure one node to read
    // We read the value of the ServerStatus -> CurrentTime
    nodeToRead.create(1);
    /*nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.Identifier.Numeric = OpcUaId_Server_ServerStatus_CurrentTime;*/

    nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.NamespaceIndex = 3;
    nodeToRead[0].NodeId.Identifier.Numeric = 1007;

    printf("\nReading ...\n");
    result = m_pSession->read(
        serviceSettings,
        0,
        OpcUa_TimestampsToReturn_Both,
        nodeToRead,
        values,
        diagnosticInfos);

    if (result.isGood())
    {
        // Read service succeded - check status of read value
        if (OpcUa_IsGood(values[0].StatusCode))
        {
            UaString name;
            name = UaVariant(values[0].Value).toString().toUtf8();
            std:cout << "Extra line: " << name << std::endl;
            UaVariant(values[0].Value).toBool(Loop);
            printf("ServerStatusCurrentTime: %s\n", UaVariant(values[0].Value).toString().toUtf8());
        }
        else
        {
            printf("Read failed for item[0] with status %s\n", UaStatus(values[0].StatusCode).toString().toUtf8());
        }
    }
    else
    {
        // Service call failed
        printf("Read failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

UaStatus SampleClient::write()
{
    UaStatus          result;
    ServiceSettings   serviceSettings;
    UaWriteValues     nodeToWrite;
    UaStatusCodeArray results;
    UaDiagnosticInfos diagnosticInfos;
    UaVariant         tempValue;

    // Configure one node to read
    // We read the value of the ServerStatus -> CurrentTime
    nodeToWrite.create(1);
    /*nodeToRead[0].AttributeId = OpcUa_Attributes_Value;
    nodeToRead[0].NodeId.Identifier.Numeric = OpcUaId_Server_ServerStatus_CurrentTime;*/

    nodeToWrite[0].AttributeId = OpcUa_Attributes_Value;
    nodeToWrite[0].NodeId.NamespaceIndex = 3;
    nodeToWrite[0].NodeId.Identifier.Numeric = 1007;
    tempValue.setDouble(96.96);
    tempValue.copyTo(&nodeToWrite[0].Value.Value);

    printf("\nWriting ...\n");
    result = m_pSession->write(
        serviceSettings,
        nodeToWrite,
        results,
        diagnosticInfos);

    if (result.isGood())
    {
        // Write service succeded - check individual status codes
        for (OpcUa_UInt32 i = 0; i < results.length(); i++)
        {
            if (OpcUa_IsGood(results[i]))
            {
                printf("Write succeeded for item[%d]\n", i);
            }
            else
            {
                printf("Write failed for item[%d] with status %s\n", i, UaStatus(results[i]).toString().toUtf8());
            }
        }
    }
    else
    {
        // Service call failed
        printf("Write failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}