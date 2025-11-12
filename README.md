#### 测试环境：imx8mp

#### 内核版本：5.15.71

##### 设备树

```
&flexspi {
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_flexspi0>;
        status = "okay";
        
        mydriver@0 {
            reg = <0>;
            compatible = "myflexdriver";
            spi-max-frequency = <133000000>;
            spi-rx-bus-width = <4>;
            spi-tx-bus-width = <4>;
        };
};

```

