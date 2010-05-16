/*
 * ClusterViewMaster.cpp
 *
 * Copyright (C) 2010 by VISUS (Universitaet Stuttgart). 
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "ClusterViewMaster.h"
#include "AbstractNamedObject.h"
#include "AbstractNamedObjectContainer.h"
#include "CallDescriptionManager.h"
#include "CalleeSlot.h"
#include "CoreInstance.h"
#include "ModuleNamespace.h"
#include "param/StringParam.h"
#include "utility/Configuration.h"
#include "view/AbstractView.h"
#include "view/CallRenderView.h"
#include "vislib/assert.h"
#include "vislib/Log.h"
#include "vislib/StringTokeniser.h"
#include "vislib/SystemInformation.h"
#include "vislib/NetworkInformation.h"

using namespace megamol::core;


/*
 * cluster::ClusterViewMaster::ClusterViewMaster
 */
cluster::ClusterViewMaster::ClusterViewMaster(void) : Module(),
        ClusterControllerClient::Listener(), ccc(), ctrlServer(),
        viewNameSlot("viewname", "The name of the view to be used"),
        viewSlot("view", "The view to be used (this value is set automatically"),
        serverAddressSlot("serverAddress", "The TCP/IP address of the server including the port"),
        serverEndPoint() {

    this->ccc.AddListener(this);
    this->MakeSlotAvailable(&this->ccc.RegisterSlot());
    this->ctrlServer.AddListener(this);

    this->viewNameSlot << new param::StringParam("");
    this->viewNameSlot.SetUpdateCallback(&ClusterViewMaster::onViewNameChanged);
    this->MakeSlotAvailable(&this->viewNameSlot);

    this->viewSlot.SetCompatibleCall<view::CallRenderViewDescription>();
    // TODO: this->viewSlot.SetVisibility(false);
    this->MakeSlotAvailable(&this->viewSlot);

    this->serverAddressSlot << new param::StringParam("");
    this->serverAddressSlot.SetUpdateCallback(&ClusterViewMaster::onServerAddressChanged);
    this->MakeSlotAvailable(&this->serverAddressSlot);

    // TODO: Implement

}


/*
 * cluster::ClusterViewMaster::~ClusterViewMaster
 */
cluster::ClusterViewMaster::~ClusterViewMaster(void) {
    this->Release();

    // TODO: Implement

}


/*
 * cluster::ClusterViewMaster::create
 */
bool cluster::ClusterViewMaster::create(void) {
    this->serverAddressSlot.Param<param::StringParam>()->SetValue(this->defaultServerAddress());

    // TODO: Implement

    return true;
}


/*
 * cluster::ClusterViewMaster::release
 */
void cluster::ClusterViewMaster::release(void) {
    this->ctrlServer.Stop();

    // TODO: Implement

}


/*
 * cluster::ClusterViewMaster::onViewNameChanged
 */
bool cluster::ClusterViewMaster::onViewNameChanged(param::ParamSlot& slot) {
    using vislib::sys::Log;
    if (!this->viewSlot.ConnectCall(NULL)) { // disconnect old call
        Log::DefaultLog.WriteMsg(Log::LEVEL_WARN,
            "Unable to disconnect call from slot \"%s\"\n",
            this->viewSlot.FullName().PeekBuffer());
    }

    CalleeSlot *viewModSlot = NULL;
    vislib::StringA viewName(this->viewNameSlot.Param<param::StringParam>()->Value());
    if (viewName.IsEmpty()) {
        // user just wanted to disconnect
        return true;
    }

    this->LockModuleGraph(false);
    AbstractNamedObject *ano = this->FindNamedObject(viewName);
    view::AbstractView *av = dynamic_cast<view::AbstractView*>(ano);
    if (av == NULL) {
        ModuleNamespace *mn = dynamic_cast<ModuleNamespace*>(ano);
        if (mn != NULL) {
            view::AbstractView *av2;
            AbstractNamedObjectContainer::ChildList::Iterator ci = mn->GetChildIterator();
            while (ci.HasNext()) {
                ano = ci.Next();
                av2 = dynamic_cast<view::AbstractView*>(ano);
                if (av2 != NULL) {
                    if (av != NULL) {
                        av = NULL;
                        break; // too many views
                    } else {
                        av = av2; // if only one view present in children, use it
                    }
                }
            }
        }
    }
    if (av != NULL) {
        viewModSlot = dynamic_cast<CalleeSlot*>(av->FindSlot("render"));
    }
    this->UnlockModuleGraph();

    if (viewModSlot == NULL) {
        Log::DefaultLog.WriteMsg(Log::LEVEL_ERROR,
            "View \"%s\" not found\n",
            viewName.PeekBuffer());
        return true; // this is just for diryt flag reset
    }

    CallDescription *cd = CallDescriptionManager::Instance()
        ->Find(view::CallRenderView::ClassName());
    if (cd == NULL) {
        Log::DefaultLog.WriteMsg(Log::LEVEL_ERROR,
            "Cannot find description for call \"%s\"\n",
            view::CallRenderView::ClassName());
        return true; // this is just for diryt flag reset
    }

    Call *c = cd->CreateCall();
    if (c == NULL) {
        Log::DefaultLog.WriteMsg(Log::LEVEL_ERROR,
            "Cannot create call \"%s\"\n",
            view::CallRenderView::ClassName());
        return true; // this is just for diryt flag reset
    }

    if (!viewModSlot->ConnectCall(c)) {
        delete c;
        Log::DefaultLog.WriteMsg(Log::LEVEL_ERROR,
            "Cannot connect call \"%s\" to inbound-slot \"%s\"\n",
            view::CallRenderView::ClassName(),
            viewModSlot->FullName().PeekBuffer());
        return true; // this is just for diryt flag reset
    }

    if (!this->viewSlot.ConnectCall(c)) {
        delete c;
        Log::DefaultLog.WriteMsg(Log::LEVEL_ERROR,
            "Cannot connect call \"%s\" to outbound-slot \"%s\"\n",
            view::CallRenderView::ClassName(),
            this->viewSlot.FullName().PeekBuffer());
        return true; // this is just for diryt flag reset
    }

    // TODO: Implement

    return true;
}


/*
 * cluster::ClusterViewMaster::OnClusterUserMessage
 */
void cluster::ClusterViewMaster::OnClusterUserMessage(cluster::ClusterControllerClient& sender,
        const cluster::ClusterController::PeerHandle& hPeer, bool isClusterMember,
        const UINT32 msgType, const BYTE *msgBody) {

    switch (msgType) {
        case ClusterControllerClient::USRMSG_QUERYHEAD:
            if (isClusterMember && this->ctrlServer.IsRunning()) {
                vislib::StringA address = this->serverEndPoint.ToStringA();
                try {
                    sender.SendUserMsg(ClusterControllerClient::USRMSG_HEADHERE,
                        reinterpret_cast<const BYTE*>(address.PeekBuffer()), address.Length() + 1);
                } catch(...) {
                }
            }
            break;
    }

}


/*
 * cluster::ClusterViewMaster::OnControlChannelMessage
 */
void cluster::ClusterViewMaster::OnControlChannelMessage(cluster::ControlChannelServer& server, cluster::ControlChannel& channel, const vislib::net::AbstractSimpleMessage& msg) {
    vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_INFO,
        "Message received %u\n", static_cast<unsigned int>(msg.GetHeader().GetMessageID()));

    // TODO: Implement

}


/*
 * cluster::ClusterViewMaster::defaultServerHost
 */
vislib::TString cluster::ClusterViewMaster::defaultServerHost(void) const {
    using vislib::net::NetworkInformation;
    const utility::Configuration& cfg = this->GetCoreInstance()->Configuration();
    if (cfg.IsConfigValueSet("cmvhost")) {
        return cfg.ConfigValue("cmvhost");
    }

    NetworkInformation::AdapterList adapters;
    NetworkInformation::GetAdaptersForType(adapters, NetworkInformation::Adapter::TYPE_ETHERNET);
    while (adapters.Count() > 0) {
        if (adapters[0].GetStatus() != NetworkInformation::Adapter::OPERSTATUS_UP) {
            adapters.RemoveFirst();
        } else break;
    }
    if (adapters.Count() > 0) {
        NetworkInformation::UnicastAddressList ual = adapters[0].GetUnicastAddresses();
        for (SIZE_T i = 0; ual.Count(); i++) {
            if (ual[i].GetAddressFamily() == vislib::net::IPAgnosticAddress::FAMILY_INET) {
                return ual[i].GetAddress()
#if defined(UNICODE) || defined(_UNICODE)
                    .ToStringW();
#else /* defined(UNICODE) || defined(_UNICODE) */
                    .ToStringA();
#endif /* defined(UNICODE) || defined(_UNICODE) */
            }
        }
        if (ual.Count() > 0) {
            vislib::TString host(ual[0].GetAddress()
#if defined(UNICODE) || defined(_UNICODE)
                .ToStringW());
#else /* defined(UNICODE) || defined(_UNICODE) */
                .ToStringA());
#endif /* defined(UNICODE) || defined(_UNICODE) */
            if (ual[0].GetAddressFamily() == vislib::net::IPAgnosticAddress::FAMILY_INET6) {
                host.Prepend(_T("["));
                host.Append(_T("]"));
            }
            return host;
        }
    }

#if defined(UNICODE) || defined(_UNICODE)
    return vislib::sys::SystemInformation::ComputerNameW();
#else /* defined(UNICODE) || defined(_UNICODE) */
    return vislib::sys::SystemInformation::ComputerNameA();
#endif /* defined(UNICODE) || defined(_UNICODE) */
}


/*
 * cluster::ClusterViewMaster::defaultServerPort
 */
unsigned short cluster::ClusterViewMaster::defaultServerPort(void) const {
    const utility::Configuration& cfg = this->GetCoreInstance()->Configuration();
    try {
        if (cfg.IsConfigValueSet("cmvport")) {
            return static_cast<unsigned short>(vislib::CharTraitsW::ParseInt(
                cfg.ConfigValue("cmvport")));
        }
    } catch(...) {
        vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_WARN,
            "Unable to parse configuration value \"cmvport\" as int. Configuration value ignored.");
    }
    return 17126;
}


/*
 * cluster::ClusterViewMaster::defaultServerAddress
 */
vislib::TString cluster::ClusterViewMaster::defaultServerAddress(void) const {
    const utility::Configuration& cfg
        = this->GetCoreInstance()->Configuration();

    if (cfg.IsConfigValueSet("cmvaddress")) { // host and port
        return cfg.ConfigValue("cmvaddress");
    }

    vislib::TString address;
    address.Format(_T("%s:%u"), this->defaultServerHost().PeekBuffer(), this->defaultServerPort());
    return address;
}


/*
 * cluster::ClusterViewMaster::onServerAddressChanged
 */
bool cluster::ClusterViewMaster::onServerAddressChanged(param::ParamSlot& slot) {
    ASSERT(&slot == &this->serverAddressSlot);
    vislib::StringA address(this->serverAddressSlot.Param<param::StringParam>()->Value());

    if (address.IsEmpty()) {
        if (this->ctrlServer.IsRunning()) {
            this->ctrlServer.Stop();
        }
        return true;
    }

    vislib::net::IPEndPoint ep;
    float wildness = vislib::net::NetworkInformation::GuessLocalEndPoint(ep, address);

    if (ep == this->serverEndPoint) return true;
    this->serverEndPoint = ep;

    if (this->ctrlServer.IsRunning()) {
        this->ctrlServer.Stop();
    }

    if (wildness > 0.8) {
        vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
            "Guessed server end point \"%s\" from \"%s\" with too high wildness: %f\n",
            ep.ToStringA().PeekBuffer(), address.PeekBuffer(), wildness);
        return true;
    }

    vislib::sys::Log::DefaultLog.WriteMsg((wildness > 0.3) ? vislib::sys::Log::LEVEL_WARN : vislib::sys::Log::LEVEL_INFO,
        "Starting server on \"%s\" guessed from \"%s\" with wildness: %f\n",
        ep.ToStringA().PeekBuffer(), address.PeekBuffer(), wildness);

    this->ctrlServer.Start(this->serverEndPoint);

    return true;
}


///*
// * cluster::ClusterViewMaster::OnCtrlCommAddressChanged
// */
//void cluster::ClusterViewMaster::OnCtrlCommAddressChanged(const vislib::TString& address) {
//    if (this->commCtrlServer.IsRunning()) {
//        this->commCtrlServerShutdown = true;
//        this->commCtrlServer.Terminate(false);
//        this->commCtrlServerShutdown = false;
//    }
//    this->stopCtrlComm();
//
//    vislib::net::IPEndPoint ep;
//    float wildness = vislib::net::NetworkInformation::GuessLocalEndPoint(ep, address);
//
//    if (wildness > 0.8f) {
//        vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
//            "Wildness for server end-point \"%s\" from input \"%s\" is too large: %f > 0.8\n",
//            ep.ToStringA().PeekBuffer(), vislib::StringA(address).PeekBuffer(), wildness);
//        return;
//    }
//
//    vislib::SmartRef<vislib::net::TcpCommChannel> c = this->startCtrlCommServer();
//
//    if (!c.IsNull()) {
//        vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_INFO,
//            "Server started at end-point \"%s\" guessed from \"%s\" with wildness %f\n",
//            ep.ToStringA().PeekBuffer(), vislib::StringA(address).PeekBuffer(), wildness);
//    } else {
//        vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
//            "Failed to start server at end-point \"%s\" guessed from \"%s\" with wildness %f\n",
//            ep.ToStringA().PeekBuffer(), vislib::StringA(address).PeekBuffer(), wildness);
//        return;
//    }
//
//    this->commCtrlServer.Configure(c.operator->(), ep.ToStringA());
//    this->commCtrlServer.Start(NULL);
//
//}


///*
// * cluster::ClusterViewMaster::OnServerExited
// */
//void cluster::ClusterViewMaster::OnServerExited(const vislib::net::CommServer& src) throw() {
//    vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_INFO, "Server exited\n");
//}
//
//
///*
// * cluster::ClusterViewMaster::OnServerStarted
// */
//void cluster::ClusterViewMaster::OnServerStarted(const vislib::net::CommServer& src) throw() {
//    vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_INFO, "Server started\n");
//}
