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
#include "uaplatformlayer.h"
#include "sampleclient.h"

/*============================================================================
 * main
 *===========================================================================*/
#ifdef _WIN32_WCE
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPWSTR, int)
#else
int main(int, char*[])
#endif
{
    UaStatus status;

    // Initialize the UA Stack platform layer
    UaPlatformLayer::init();

    // Create instance of SampleClient
    SampleClient* pMyClient = new SampleClient();

    // Connect to OPC UA Server
    status = pMyClient->connect();

    // Connect succeeded
    if (status.isGood())
    {
        // Read values
        status = pMyClient->read();

        printf("\nPress Enter to write\n");
        getchar();

        status = pMyClient->write();

        printf("\nPress Enter to read again\n");
        getchar();

        status = pMyClient->read();

        // Wait for user command.
        printf("\nPress Enter to disconnect\n");
        getchar();

        // Disconnect from OPC UA Server
        status = pMyClient->disconnect();
    }

    // Close application
    printf("\nPress Enter to close\n");
    // Wait for user command.
    getchar();

    delete pMyClient;
    pMyClient = NULL;

    // Cleanup the UA Stack platform layer
    UaPlatformLayer::cleanup();

    return 0;
}
