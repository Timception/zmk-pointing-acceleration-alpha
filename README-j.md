[ [English](README.md) | Japanese ]

[テンプレート](https://github.com/oleksandrmaslov/zmk-pointing-acceleration)からリポジトリを作成しました。
**元のソースに基づいてテンプレートを作成してください。**
**必要に応じて、`Use this Template`からリポジトリを自由に作成してください。**

**NUOVOTAKA による修正** - コード品質の向上、エラーハンドリングの改善、保守性の向上。

---

<div align="center">
  <h1>ZMK マルチレベル ポインティング アクセラレーション</h1>
    <a href="https://badgen.net/badge/ZMK/v0.3.0/blue" alt="ZMK">
      <img src="https://badgen.net/badge/ZMK/v0.3.0/blue" />
    </a>
    <a href="https://badgen.net/badge/license/MIT" alt="License">
      <img src="https://badgen.net/badge/license/MIT" />
    </a>
    <a href="https://deepwiki.com/nuovotaka/zmk-pointing-acceleration-alpha" alt="Ask DeepWiki">
        <img src="https://deepwiki.com/badge.svg" />
    </a>
<hr>
<p align="center">

このリポジトリは、初心者から上級者まで対応した<br> ZMK 用の**2 レベル**ポインターアクセラレーションシステムを提供します。
<br><br>
アクセラレーションは低速時の精密なカーソル移動を向上させ、高速時の迅速な移動を可能にします。<br>アクセラレーションカーブはカスタマイズ可能で、ポインティングデバイスに合わせて設定を調整できます。

</p>

</div>

---

## 🎯 **レベルを選択**

### **レベル 1: シンプル** (ほとんどのユーザーに推奨)

- **3 つのポインティングデバイスタイプ**と**4 つの感度レベル**に対応した**5 つの設定可能項目**
- 迅速で簡単なセットアップ
- 異なるセンサーに対する自動 DPI 調整
- オフィス、ゲーミング、高感度のプリセット（光学、レーザー、トラックボール、トラックパッド対応）

### **レベル 2: スタンダード** (高度な機能)

- **3 つのポインティングデバイスタイプ**と**4 つの感度レベル**に対応した**8 つの設定可能項目**
- 速度依存アクセラレーションと Y 軸ブースト
- 高度な指数カーブオプション（1-5）
- 完全なカスタマイズとデバイス固有の調整を求めるユーザーに最適

**デバイス互換性:** PAW3222 トラックボールと Azoteq IQS7211E トラックパッドセンサーでテスト済み。他のデバイスでも動作するはずですが、センサーの違いがパフォーマンスに影響する可能性があります。

**開始前に、入力デバイスが動作することを確認してください: https://zmk.dev/docs/features/pointing**

## ✨ レベル別機能

### **レベル 1: シンプル機能**

- ✅ **プリセット設定** (オフィス、ゲーミング、高感度)
- ✅ **基本感度**調整
- ✅ **3 つのカーブタイプ** (リニア、マイルド、ストロング)
- ✅ **Y 軸ブースト** ディスプレイ最適化用
- ✅ **プラグアンドプレイ**セットアップ

### **レベル 2: スタンダード機能**

- ✅ レベル 1 の全機能**（プリセット含む）**
- ✅ 設定可能な閾値を持つ**速度ベースアクセラレーション**
- ✅ ワイドスクリーンディスプレイ用**Y 軸ブースト**
- ✅ 数学的精度を持つ**高度な指数カーブ**:
  - **リニアカーブ (1)**: `f(t) = t` - 一定のアクセラレーション率
  - **指数カーブ (2-5)**: 自然で滑らかなアクセラレーション感
    - マイルド指数 (2): `f(t) = e^(2t) - 1` - 一般用途にバランス良好
    - モデレート指数 (3): `f(t) = e^(3t) - 1` - より反応的
    - ストロング指数 (4): `f(t) = e^(4t) - 1` - アグレッシブなアクセラレーション
    - アグレッシブ指数 (5): `f(t) = e^(5t) - 1` - 最大の反応性
- ✅ 完全なカスタマイズのための**8 つの設定可能パラメータ**
- ✅ あらゆる相対入力デバイス（ポインティングデバイス、トラックボール、タッチパッド）と互換

## 🔍 **レベル差の理解**

**重要:** レベル 1 とレベル 2 は完全に異なる計算方法を使用します。同じプリセット名でもレベル間で動作が異なります。

📖 **[詳細なレベル比較ガイドを読む →](docs/LEVEL_COMPARISON-j.md)**  
📋 **[全プリセット設定を表示 →](docs/PRESETS-j.md)**

### **クイックサマリー**

- **レベル 1**: 入力ベース、即座の反応、ゲーミングスタイルアクセラレーション
- **レベル 2**: 速度ベース、段階的反応、自然で滑らかなアクセラレーション

予測可能な即座のアクセラレーションにはレベル 1 を、適応的で滑らかなアクセラレーションにはレベル 2 を選択してください。

## インストールと使用方法

ポインターアクセラレーションを使用するには、いくつかのステップが必要です:

- `west.yml`を調整してアクセラレーションモジュールを利用可能にする
- 設定ファイルに依存関係をインポートする
- アクセラレーションパラメータを設定する
- 入力チェーンにアクセラレーションプロセッサを追加する

これらのステップを順番に説明します。

### west.yml の調整

`west.yml`にアクセラレーションモジュールを追加:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: nuovotaka
      url-base: https://github.com/nuovotaka
  projects:
    - name: zmk-pointing-acceleration-alpha
      remote: nuovotaka
      revision: main
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
```

> [!WARNING]  
> ローカルマシンでビルドしている場合（GitHub Actions ではない場合）は`west update`を実行してください。

### 依存関係のインポート

デバイスオーバーレイファイル（例：`yourkeyboard_left.overlay`）に必要なインクルードを追加:

```C
#include <input/processors.dtsi>
#include <behaviors/input_gestures_accel.dtsi>
```

### アクセラレーションモジュールの有効化

**重要:** `prj.conf`ファイルに必要な設定を追加:

```ini
# 必須: アクセラレーションモジュールを有効化
CONFIG_ZMK_INPUT_PROCESSOR_ACCELERATION=y
```

### レベルの設定

`prj.conf`ファイルで設定レベルを選択:

```ini
# レベルを選択 (1 または 2)
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y      # レベル1: シンプル
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y  # レベル2: スタンダード
```

### アクセラレーションの設定

デバイスオーバーレイにアクセラレーション設定を追加。この設定は*prj.conf*に配置する必要があります。

#### **レベル 1: シンプル設定**

**オプション A: プリセットを使用（推奨）**

利用可能なプリセットはレベル 1 とレベル 2 の両方で動作します（12 のデバイス固有プリセット）:

<details>
<summary style="cursor:pointer; font-weight:bold;">プリセットの設定</summary>

```ini
# prj.confで、デバイス固有のプリセットを選択:

# オフィスプリセット:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL=y     # 光学マウス (800 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_LASER=y     # レーザーマウス (1600 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKBALL=y # トラックボール (400 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKPAD=y  # トラックパッド (1000 DPI)

# ゲーミングプリセット:
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL=y   # 光学マウス (1200 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER=y     # レーザーマウス (3200 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKBALL=y # トラックボール (800 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKPAD=y  # トラックパッド (1200 DPI)

# 高感度プリセット:
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_OPTICAL=y   # 光学マウス (1600 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_LASER=y     # レーザーマウス (6400 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKBALL=y # トラックボール (800 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKPAD=y  # トラックパッド (1200 DPI)
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    // プリセット値が自動的に適用されます！
};
```

</details>

**オプション B: カスタムシンプル設定**

> **注意**: レベル 1 のカスタム設定は初心者向けに設計されており、過度なカーソル移動を防ぐために控えめなアクセラレーション制限を使用します。システムは意図的に最大アクセラレーションを 3.0 倍に制限し、予測可能な動作のために適度な速度閾値を使用します。より積極的なアクセラレーションが必要な場合は、まずレベル 2 プリセットを試し、その後レベル 2 カスタム設定を検討してください。

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```ini
# prj.confで:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1300>;     // 1.3倍基本感度
    max-factor = <2800>;      // 2.8倍最大アクセラレーション（安全のため3.0倍でキャップ）
    curve-type = <1>;         // 0=リニア、1=マイルド、2=ストロング
    y-boost = <1200>;         // 1.2倍Y軸ブースト（オプション、デフォルト1000）
    sensor-dpi = <800>;       // 800 DPIセンサー（オプション、デフォルト800）
};
```

</details>

#### **レベル 2: スタンダード設定**

**オプション A: プリセットを使用（推奨）**

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```ini
# prj.confで:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER=y       # ゲーミングレーザーマウス
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL=y   # オフィス光学マウス
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKBALL=y # 高感度トラックボール
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    // 全レベル2設定のプリセット値が自動的に適用されます！
    // 含まれる設定: sensitivity, max-factor, acceleration-exponent, y-boost,
    // speed-threshold, speed-max, min-factor
};
```

</details>

**オプション B: カスタムスタンダード設定**

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```ini
# prj.confで:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1200>;         // 1.2倍基本感度
    max-factor = <3000>;          // 3.0倍最大アクセラレーション
    acceleration-exponent = <2>;  // 高度な指数カーブ (1-5)
    y-boost = <1300>;             // ワイドスクリーン用1.3倍Y軸ブースト
    speed-threshold = <600>;      // 600カウント/秒でアクセラレーション開始
    speed-max = <3500>;           // 3500カウント/秒で最大アクセラレーション
    min-factor = <1000>;          // 1.0倍最小値（減速なし）
    sensor-dpi = <800>;           // 800 DPIセンサー（オプション、デフォルト800）
};
```

</details>

### 入力チェーンに追加

入力デバイスのプロセッサチェーンにアクセラレーションプロセッサを追加:

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
/ {
    tpad0: tpad0 {
        compatible = "zmk,input-listener";
        status = "okay";
        device = <&glidepoint>;
        input-processors = <
            &pointer_accel      // アクセラレーションプロセッサ
            &zip_xy_transform
        >;
    };
};
```

</details>

## 設定オプション

アクセラレーションプロセッサは、ポインティングデバイスの動作をカスタマイズするためのいくつかの設定を提供します。各オプションの詳細な説明は以下の通りです:

### 基本設定

- `sensitivity`: (デフォルト: 1000)

  - 全ポインター移動の基本感度倍率を制御
  - 値は千分率（例：1200 = 1.2 倍速度、800 = 0.8 倍速度）
  - アクセラレーションが適用される前のベースライン速度に影響
  - **低い値（例：800）**: より遅く、より精密な移動 - 詳細作業に適している
  - **高い値（例：1500）**: より速い移動 - 大型ディスプレイや迅速なナビゲーションに適している
  - 例: `sensitivity = <1200>`は全ての移動をデフォルトより 20%速くします

- `max-factor`: (デフォルト: 3500)
  - 高速時の最大アクセラレーションを制御
  - 値は千分率（例：3500 = 3.5 倍速度）
  - **低い値（例：2000）**: より控えめなアクセラレーション - より予測可能な移動
  - **高い値（例：5000）**: より積極的なアクセラレーション - 長距離移動により速い
  - 例: `max-factor = <3000>`は高速移動が最大 3 倍速くなることを意味します

### 速度設定

- `speed-threshold`: (デフォルト: 1000)

  - アクセラレーションが開始される速度
  - 秒あたりカウント数で測定
  - この速度以下では基本感度が適用
  - この速度以上でアクセラレーションが開始
  - 例: `speed-threshold = <1200>`は中程度の速度でアクセラレーションが開始されることを意味します

- `speed-max`: (デフォルト: 6000)
  - 最大アクセラレーションに達する速度
  - 秒あたりカウント数で測定
  - この速度以上で max-factor が適用
  - 例: `speed-max = <6000>`は高速で最大アクセラレーションに達することを意味します

### アクセラレーション動作

- `min-factor`: (デフォルト: 1000) **[レベル 2 スタンダードのみ]**

  - 低速移動の処理方法を制御
  - 1000 未満の値は精密性のために低速移動をさらに遅くします
  - 値は千分率（例：800 = 0.8 倍速度）
  - 例: `min-factor = <800>`は低速移動を 20%遅くします

- `acceleration-exponent`: (デフォルト: 2) **[レベル 2 スタンダードのみ]**
  - アクセラレーションの高度な数学的カーブタイプを制御
  - **利用可能なカーブ:**
    - 1 = リニア `t` (一定のアクセラレーション率)
    - 2 = マイルド指数 `e^(2t) - 1` (デフォルト、バランス良好)
    - 3 = モデレート指数 `e^(3t) - 1` (より反応的)
    - 4 = ストロング指数 `e^(4t) - 1` (積極的)
    - 5 = アグレッシブ指数 `e^(5t) - 1` (最大の反応性)
  - 例: `acceleration-exponent = <3>`はモデレート指数アクセラレーション用

### ハードウェア設定

- `sensor-dpi`: (デフォルト: 800)
  - ポインティングデバイスセンサーの DPI/CPI 設定
  - 自動感度スケーリングに使用
  - 一般的な値: 400、800、1200、1600、3200 DPI
  - 高 DPI センサーは一貫した感触を維持するために感度が低下します
  - 例: `sensor-dpi = <1600>`は 1600 DPI センサー用

### 視覚的例

異なる設定がポインター移動にどのように影響するかの例:

```
低速度    │  中速度     │  高速度
─────────┼────────────┼────────────
0.8倍   →│    1.0倍   →│   3.0倍     (バランス)
0.9倍   →│    1.0倍   →│   2.0倍     (ライト)
0.7倍   →│    1.0倍   →│   4.0倍     (ヘビー)
0.5倍   →│    1.0倍   →│   1.5倍     (精密)
```

## 設定を共有

### 簡単な設定可視化アプリ: https://pointing.streamlit.app/

以下の設定は単なる出発点です - 完璧なポインター設定は人それぞれユニークです。あなたに最適な設定を見てみたいです。

### なぜ共有するのか？

- 他の人が理想的なセットアップを見つけるのを助ける
- コミュニティの知識に貢献する
- フィードバックと提案を得る
- 新しい設定アイデアを刺激する

### 共有方法

- GitHub イシューに設定を投稿
- ZMK Discord または DM（使用ケースの簡単なメモ付き）で共有
- 何が効果的だった/効果的でなかったかコメント

> **注意**: これらの例は主に PAW3222 ポインティングデバイスセンサーを使用したトラックボールでテストされました。他のポインティングデバイス（トラックパッドやトラックポイントなど）を使用している場合、結果は異なる可能性があります - だからこそあなたの経験を共有することが非常に価値があるのです

### 一般用途 (レベル 2 スタンダード):

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント
    sensitivity = <1200>;      // 1.2倍基本感度
    max-factor = <3000>;       // 大きな移動に適したアクセラレーション
    y-boost = <1200>;          // 1.2倍Y軸ブースト
    speed-threshold = <1200>;  // バランスの取れたアクセラレーションポイント
    speed-max = <6000>;        // 6000カウント/秒で最大アクセラレーション
    min-factor = <800>;        // 精密性のための0.8倍最小値
    acceleration-exponent = <2>; // マイルド指数カーブ
    sensor-dpi = <800>;        // 800 DPIセンサー
};
```

</details>

### ライトアクセラレーション (レベル 2 スタンダード):

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント
    sensitivity = <1100>;      // 1.1倍基本感度
    max-factor = <2000>;       // 2.0倍最大値
    y-boost = <1100>;          // 1.1倍Y軸ブースト
    speed-threshold = <1500>;  // より遅くアクセラレーション開始
    speed-max = <5000>;        // 最大アクセラレーションまで5000カウント/秒
    acceleration-exponent = <1>; // リニアアクセラレーション
    sensor-dpi = <800>;        // 800 DPIセンサー
};
```

</details>

### ヘビーアクセラレーション (レベル 2 スタンダード):

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント
    sensitivity = <1000>;      // 1.0倍基本感度
    max-factor = <4000>;       // 4.0倍最大値
    y-boost = <1000>;          // 1.0倍Y軸ブースト
    speed-threshold = <1000>;  // より早くアクセラレーション開始
    speed-max = <6000>;        // 最大アクセラレーションまで6000カウント/秒
    min-factor = <700>;        // 精密性のための0.7倍最小値
    acceleration-exponent = <4>; // ストロング指数カーブ
    sensor-dpi = <800>;        // 800 DPIセンサー
};
```

</details>

### ワイドスクリーン最適化 (レベル 2 スタンダード) - テスト済み設定:

#### ゲーミング

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント
    sensitivity = <800>;       // 0.8倍基本感度
    max-factor = <2500>;       // 2.5倍最大アクセラレーション
    y-boost = <1000>;          // ワイドスクリーン用2.5倍Y軸ブースト
    speed-threshold = <300>;   // 早期アクセラレーション開始
    speed-max = <3000>;        // 最大アクセラレーションのための低速度
    min-factor = <800>;        // 精密性のための0.8倍最小値
    acceleration-exponent = <3>; // モデレート指数カーブ
    sensor-dpi = <800>;        // 800 DPIセンサー
};
```

</details>

#### オフィス

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント

    /* ------------- オフィスプリセット ------------- */
    /* 低い基本感度、非常に穏やかなアクセラレーション */
    /* 遅く精密なポインター移動を優先               */

    sensitivity           = <600>;   /* 0.6 × 基本感度 */
    min-factor            = <700>;   /* 0.7×から開始 */
    max-factor            = <1500>;  /* 最大1.5×アクセラレーション */
    speed-threshold       = <400>;   /* 400 cnt/sでアクセラレーション開始 */
    speed-max             = <2500>;  /* 2500 cnt/sでmax-factorに到達 */
    y-boost               = <1000>;  /* XとY軸で同じ倍率 */
    acceleration-exponent = <1>;     /* リニア（指数カーブなし） */
    sensor-dpi            = <800>;   /* 800 DPIセンサーを想定 */
};
```

</details>

#### 高感度

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント

    /* --- 高感度: 低速でブーストなし、中/高速で強いブースト --- */
    /* 初期移動は精密（アクセラレーションなし）、中/高速でアクセラレーション増加 */

    sensitivity           = <1000>;   /* 1.0 × 基本感度（低速でゲインなし） */
    min-factor            = <1000>;   /* 低速で1.0×（アクセラレーションなし） */
    max-factor            = <4000>;   /* 高速で最大4.0× */
    speed-threshold       = <300>;    /* 300 cnt/s以上でアクセラレーション開始 */
    speed-max             = <1200>;   /* 1200 cnt/sでmax-factorに到達 */
    y-boost               = <1300>;   /* Y軸1.3×（Xに合わせる） */
    acceleration-exponent = <4>;      /* 中/高速用ストロング指数 */
    sensor-dpi            = <800>;
};
```

</details>

### 精密モード (レベル 1 シンプル):

<details>
<summary style="cursor:pointer; font-weight:bold;">サンプルコード</summary>

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // XとY軸イベント
    sensitivity = <800>;       // 細かい制御のための0.8倍
    max-factor = <1500>;       // 1.5倍最大値
    curve-type = <0>;          // リニアカーブ
    y-boost = <1000>;          // 精密性のための1.0倍Y軸（ブーストなし）
    sensor-dpi = <800>;        // 800 DPIセンサー
};
```

</details>
