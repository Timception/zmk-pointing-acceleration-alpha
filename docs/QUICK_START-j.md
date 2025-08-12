[ [English](QUICK_START.md) | Japanese ]

# クイックスタートガイド

## 1. レベルを選択

- **レベル 1（シンプル）**: 5 つの基本設定 + プリセット - ほとんどのユーザーに推奨
  - 予測可能な動作のための**控えめなアクセラレーション制限**
  - 過度な移動を防ぐ安全キャップ付きの**初心者向け**
- **レベル 2（スタンダード）**: 速度ベースアクセラレーションと高度なカーブを持つ 8 つの設定
  - 積極的なアクセラレーションオプションが利用可能な**完全制御**

## 主要設定概要

### 基本パラメータ（両レベル）

- `sensitivity`: 基本速度倍率（1000 = 1.0 倍、1200 = 1.2 倍）
  - **低い値**: より精密で遅い移動
  - **高い値**: 大型ディスプレイ用のより速い移動
- `max-factor`: 高速時の最大アクセラレーション
- `curve-type`: アクセラレーションカーブ（0=リニア、1=マイルド、2=ストロング）
- `y-boost`: Y 軸感度倍率（1000 = 1.0 倍、1200 = 1.2 倍 Y 軸）
  - **ワイドスクリーンディスプレイに有用** - 垂直移動のブーストが必要な場合

## 2. 基本セットアップ

### west.yml に追加

```yaml
manifest:
  remotes:
    - name: nuovotaka
      url-base: https://github.com/nuovotaka
  projects:
    - name: zmk-pointing-acceleration-alpha
      remote: nuovotaka
      revision: main
```

### オーバーレイファイルに追加

```c
#include <input/processors.dtsi>
#include <behaviors/input_gestures_accel.dtsi>
```

### prj.conf で設定

```ini
# 1つのレベルを選択
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y      # レベル1
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y  # レベル2

# デバイス固有のプリセットを選択（両レベルで動作）
# レベル1（シンプル）用:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL=y
# レベル2（スタンダード）用:
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER=y
```

## 3. 入力チェーンに追加

```devicetree
/ {
    tpad0: tpad0 {
        compatible = "zmk,input-listener";
        device = <&glidepoint>;
        input-processors = <&pointer_accel>;
    };
};
```

## 4. テストと調整

デフォルトから開始し、好みに基づいて微調整します。

詳細な設定オプションについては、[CONFIGURATION_EXAMPLES.md](CONFIGURATION_EXAMPLES.md)を参照してください。
