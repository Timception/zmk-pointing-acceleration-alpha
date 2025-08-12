[ [English](CONFIGURATION_EXAMPLES.md) | Japanese ]

# 設定例

## レベル 1: シンプル設定

### プリセットの使用（推奨）

利用可能なデバイス固有プリセット:

```ini
# prj.confで
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y

# デバイス固有のプリセットを選択（1つだけ有効にする）:

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
    // プリセット値が自動的に適用されます
};
```

### カスタムシンプル設定

> **重要**: レベル 1 のカスタム設定は、控えめなアクセラレーション制限を持つ初心者向けに設計されています。システムは最大アクセラレーションを 3.0 倍でキャップし、適度な速度閾値を使用することで過度なカーソル移動を防ぎます。より積極的なアクセラレーションが必要な場合は、まずレベル 2 プリセットを試し、その後レベル 2 カスタム設定を検討してください。

```ini
# prj.confで
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1300>;     // 1.3倍基本感度
    max-factor = <2800>;      // 2.8倍最大アクセラレーション（3.0倍でキャップ）
    curve-type = <1>;         // マイルドカーブ
    y-boost = <1200>;         // 1.2倍Y軸ブースト（オプション）
    sensor-dpi = <800>;       // 800 DPIセンサー（オプション）
};
```

## レベル 2: スタンダード設定

### プリセットの使用（推奨）

レベル 2 で利用可能なデバイス固有プリセット:

```ini
# prj.confで
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y

# デバイス固有のプリセットを選択（1つだけ有効にする）:

# オフィスプリセット:
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL=y   # 光学マウス (800 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_LASER=y     # レーザーマウス (1600 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKBALL=y # トラックボール (400 DPI)
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKPAD=y  # トラックパッド (1000 DPI)

# ゲーミングプリセット:
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL=y   # 光学マウス (1200 DPI)
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER=y       # レーザーマウス (3200 DPI)
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
    // 全レベル2設定のプリセット値が自動的に適用されます
    // 含まれる設定: sensitivity, max-factor, acceleration-exponent, y-boost,
    // speed-threshold, speed-max, min-factor
};
```

### カスタムスタンダード設定

```ini
# prj.confで
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1200>;         // 1.2倍基本感度
    max-factor = <3000>;          // 3.0倍最大アクセラレーション
    acceleration-exponent = <2>;  // 指数カーブ (1-5)
    y-boost = <1300>;             // 1.3倍Y軸ブースト
    speed-threshold = <600>;      // 600カウント/秒でアクセラレーション開始
    speed-max = <3500>;           // 3500カウント/秒で最大アクセラレーション
    min-factor = <1000>;          // 1.0倍最小値（減速なし）
    sensor-dpi = <800>;           // 800 DPIセンサー（オプション）
};
```

## パラメータ説明

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

- `curve-type`: (デフォルト: 1)

  - アクセラレーションカーブの形状を制御
  - **0 = リニア**: 一定のアクセラレーション率 - 予測可能だが自然さに欠ける
  - **1 = マイルド**: バランスの取れたカーブ - ほとんどのユーザーに適している
  - **2 = ストロング**: 積極的なカーブ - より反応的な感触

- `y-boost`: (デフォルト: 1000) **[レベル 1・2 両方で利用可能]**
  - Y 軸感度倍率
  - 値は千分率（例：1200 = 1.2 倍 Y 軸速度）
  - **1000**: X 軸と同じ（ブーストなし）
  - **高い値（例：1300）**: より速い垂直移動 - ワイドスクリーンディスプレイに有用
  - 例: `y-boost = <1200>`は垂直移動を 20%速くします

## トラブルシューティング

### 感度が高すぎる

- `sensitivity`または`max-factor`を減少
- `speed-threshold`を増加
- リニアカーブを使用（`curve-type = <0>`）

### 感度が不十分

- `sensitivity`または`max-factor`を増加
- `speed-threshold`を減少
- ストロングカーブを使用（`curve-type = <2>`）

### ぎくしゃくした移動

- `max-factor`を減少
- レベル 1 でマイルドカーブを使用（`curve-type = <1>`）
- レベル 2 でリニア指数カーブを使用（`acceleration-exponent = <1>`）
- より良い安定性のために簡素化された処理を使用

### 小さな移動が失われる

- より良い精密性のために低い感度値を使用
- 小さな移動を保持するために`min-factor`を減少
- より早くアクセラレーションを開始するために`speed-threshold`を下げる

## テスト済み設定

### ワイドスクリーン最適化セットアップ（レベル 2）

この設定はワイドスクリーンディスプレイ用にテストされ最適化されています:

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1300>;         // 1.3倍基本感度
    max-factor = <5000>;          // 5.0倍最大アクセラレーション
    y-boost = <2500>;             // ワイドスクリーン用2.5倍Y軸ブースト
    speed-threshold = <200>;      // 早期アクセラレーション開始
    speed-max = <4000>;           // 最大アクセラレーションのための低速度
    min-factor = <800>;           // 精密性のための0.8倍最小値
    acceleration-exponent = <4>;  // ストロング指数カーブ
    sensor-dpi = <800>;           // 800 DPIセンサー
};
```

**特徴:**

- **反応的アクセラレーション** - 低速（200 カウント/秒）でアクセラレーション開始
- **強い Y 軸ブースト** - 快適な垂直移動のための 2.5 倍倍率
- **滑らかなカーブ** - 自然な感触のためのストロング指数アクセラレーション
- **簡素化された処理** - 安定性と安全性のために最適化
