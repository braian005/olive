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

#ifndef MERGENODE_H
#define MERGENODE_H

#include "node/node.h"

namespace olive {

class MergeNode : public Node
{
  Q_OBJECT
public:
  MergeNode();

  virtual Node* copy() const override;

  virtual QString Name() const override;
  virtual QString id() const override;
  virtual QVector<CategoryID> Category() const override;
  virtual QString Description() const override;

  virtual void Retranslate() override;

  virtual ShaderCode GetShaderCode(const QString &shader_id) const override;
  virtual NodeValueTable Value(const QString& output, NodeValueDatabase &value) const override;

  static const QString kBaseIn;
  static const QString kBlendIn;

  virtual void Hash(const QString& output, QCryptographicHash &hash, const rational &time) const override;

private:
  NodeInput* base_in_;

  NodeInput* blend_in_;

};

}

#endif // MERGENODE_H
