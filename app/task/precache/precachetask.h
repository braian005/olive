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

#ifndef PRECACHETASK_H
#define PRECACHETASK_H

#include "project/item/footage/footage.h"
#include "project/item/sequence/sequence.h"
#include "task/render/render.h"

namespace olive {

class PreCacheTask : public RenderTask
{
  Q_OBJECT
public:
  PreCacheTask(Footage* footage, int index, Sequence* sequence);

  virtual ~PreCacheTask() override;

protected:
  virtual bool Run() override;

  virtual void FrameDownloaded(FramePtr frame, const QByteArray& hash, const QVector<rational>& times, qint64 job_time) override;

  virtual void AudioDownloaded(const TimeRange& range, SampleBufferPtr samples, qint64 job_time) override;

private:
  Footage* footage_;

  int index_;

};

}

#endif // PRECACHETASK_H
