/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2020 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "sequence.h"

#include <QThread>

#include "panel/timeline/timeline.h"
#include "ui/icons/icons.h"

namespace olive {

const QString Sequence::kTrackInputFormat = QStringLiteral("track_in_%1");

#define super ViewerOutput

Sequence::Sequence()
{
  // Create TrackList instances
  track_lists_.resize(Track::kCount);

  for (int i=0;i<Track::kCount;i++) {
    // Create track input
    QString track_input_id = kTrackInputFormat.arg(i);

    AddInput(track_input_id, NodeValue::kNone, InputFlags(kInputFlagNotKeyframable | kInputFlagArray));

    IgnoreInvalidationsFrom(track_input_id);

    TrackList* list = new TrackList(this, static_cast<Track::Type>(i), track_input_id);
    track_lists_.replace(i, list);
    connect(list, &TrackList::TrackListChanged, this, &Sequence::UpdateTrackCache);
    connect(list, &TrackList::LengthChanged, this, &Sequence::VerifyLength);
    connect(list, &TrackList::TrackAdded, this, &Sequence::TrackAdded);
    connect(list, &TrackList::TrackRemoved, this, &Sequence::TrackRemoved);
  }
}

Sequence::~Sequence()
{
  // Should prevent traversing graph unnecessarily
  BeginOperation();
  DisconnectAll();
  EndOperation();
}

void Sequence::add_default_nodes(MultiUndoCommand* command)
{
  // Create tracks and connect them to the viewer
  command->add_child(new TimelineAddTrackCommand(track_list(Track::kVideo)));
  command->add_child(new TimelineAddTrackCommand(track_list(Track::kAudio)));
}

QIcon Sequence::icon() const
{
  return icon::Sequence;
}

QVector<Track *> Sequence::GetUnlockedTracks() const
{
  QVector<Track*> tracks = GetTracks();

  for (int i=0;i<tracks.size();i++) {
    if (tracks.at(i)->IsLocked()) {
      tracks.removeAt(i);
      i--;
    }
  }

  return tracks;
}

void Sequence::Retranslate()
{
  super::Retranslate();

  for (int i=0;i<Track::kCount;i++) {
    QString input_name;

    switch (static_cast<Track::Type>(i)) {
    case Track::kVideo:
      input_name = tr("Video Tracks");
      break;
    case Track::kAudio:
      input_name = tr("Audio Tracks");
      break;
    case Track::kSubtitle:
      input_name = tr("Subtitle Tracks");
      break;
    case Track::kNone:
    case Track::kCount:
      break;
    }

    if (!input_name.isEmpty()) {
      SetInputName(kTrackInputFormat.arg(i), input_name);
    }
  }
}

rational Sequence::GetCustomLength(Track::Type type) const
{
  if (!track_lists_.isEmpty()) {
    switch (type) {
    case Track::kVideo:
      return track_lists_.at(Track::kVideo)->GetTotalLength();
    case Track::kAudio:
      return track_lists_.at(Track::kAudio)->GetTotalLength();
    case Track::kSubtitle:
      return track_lists_.at(Track::kSubtitle)->GetTotalLength();
    case Track::kNone:
    case Track::kCount:
      break;
    }
  }

  return rational();
}

void Sequence::InputConnectedEvent(const QString &input, int element, const NodeOutput &output)
{
  foreach (TrackList* list, track_lists_) {
    if (list->track_input() == input) {
      // Return because we found our input
      list->TrackConnected(output.node(), element);
      return;
    }
  }

  super::InputConnectedEvent(input, element, output);
}

void Sequence::InputDisconnectedEvent(const QString &input, int element, const NodeOutput &output)
{
  foreach (TrackList* list, track_lists_) {
    if (list->track_input() == input) {
      // Return because we found our input
      list->TrackDisconnected(output.node(), element);
      return;
    }
  }

  super::InputDisconnectedEvent(input, element, output);
}

void Sequence::ShiftAudioEvent(const rational &from, const rational &to)
{
  foreach (Track* track, track_lists_.at(Track::kAudio)->GetTracks()) {
    track->waveform().Shift(from, to);
  }
}

void Sequence::UpdateTrackCache()
{
  track_cache_.clear();

  foreach (TrackList* list, track_lists_) {
    foreach (Track* track, list->GetTracks()) {
      track_cache_.append(track);
    }
  }
}

void Sequence::LoadInternal(QXmlStreamReader *reader, XMLNodeData &xml_node_data, uint version, const QAtomicInt *cancelled)
{
  Q_UNUSED(xml_node_data)
  Q_UNUSED(version)
  Q_UNUSED(cancelled)

  while (XMLReadNextStartElement(reader)) {
    if (reader->name() == QStringLiteral("points")) {
      timeline_points_.Load(reader);
    } else {
      reader->skipCurrentElement();
    }
  }
}

void Sequence::SaveInternal(QXmlStreamWriter *writer) const
{
  // Write TimelinePoints
  writer->writeStartElement(QStringLiteral("points"));
  timeline_points_.Save(writer);
  writer->writeEndElement(); // points
}

}
