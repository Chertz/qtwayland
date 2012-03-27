/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "wlqtkey.h"
#include "wlsurface.h"
#include <QKeyEvent>
#include <QWindow>

namespace Wayland {

static void dummy(wl_client *, wl_resource *)
{
}

const struct wl_qtkey_extension_interface QtKeyExtensionGlobal::qtkey_interface = {
    dummy
};

QtKeyExtensionGlobal::QtKeyExtensionGlobal(Compositor *compositor)
    : m_compositor(compositor)
{
    wl_display_add_global(compositor->wl_display(),
                          &wl_qtkey_extension_interface,
                          this,
                          QtKeyExtensionGlobal::bind_func);
}

QtKeyExtensionGlobal::~QtKeyExtensionGlobal()
{
}

void QtKeyExtensionGlobal::destroy_resource(wl_resource *resource)
{
    QtKeyExtensionGlobal *self = static_cast<QtKeyExtensionGlobal *>(resource->data);
    self->m_resources.removeOne(resource);
    free(resource);
}

void QtKeyExtensionGlobal::bind_func(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_resource *resource = wl_client_add_object(client, &wl_qtkey_extension_interface, &qtkey_interface, id, data);
    resource->destroy = destroy_resource;
    QtKeyExtensionGlobal *self = static_cast<QtKeyExtensionGlobal *>(resource->data);
    self->m_resources.append(resource);
}

void QtKeyExtensionGlobal::postQtKeyEvent(QKeyEvent *event, Surface *surface)
{
    wl_client *surfaceClient = surface->base()->resource.client;
    uint32_t time = m_compositor->currentTimeMsecs();
    const int rescount = m_resources.count();

    for (int res = 0; res < rescount; ++res) {
        wl_resource *target = m_resources.at(res);
        if (target->client != surfaceClient)
            continue;

        QByteArray textUtf8 = event->text().toUtf8();

        wl_qtkey_extension_send_qtkey(target,
                time, event->type(), event->key(), event->modifiers(),
                event->nativeScanCode(),
                event->nativeVirtualKey(),
                event->nativeModifiers(),
                textUtf8.constData(),
                event->isAutoRepeat(),
                event->count());
    }
}

}